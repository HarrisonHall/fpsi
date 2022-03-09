// imgui-gui.cpp

//// For reference:
//// https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html
//// Very helpful for checking how widgets are used

#include <cassert>
#include <cstring>
#include <deque>
#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <string>
#include <thread>

#include "imgui-gui.hpp"

#include "data/datahandler.hpp"
#include "data/dataframe.hpp"
#include "data/datasource.hpp"
#include "session/session.hpp"
#include "util/logging.hpp"



static void glfw_error_callback(int error, const char* description) {
	util::log(util::error, "Glfw Error %d: %s\n", error, description);
}

GLFWwindow* setup_imgui() {
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return nullptr;
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100
	const char* glsl_version = "#version 100";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
	// GL 3.2 + GLSL 150
	const char* glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
	
	// Generic imgui setup
	GLFWwindow* window = glfwCreateWindow(1280, 720, "FPSI", NULL, NULL);
	if (window == nullptr)
		return nullptr;
	
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync
	
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();
	
	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
	return window;
}


namespace fpsi {

ImGuiGUI::ImGuiGUI(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config) :
	Plugin(plugin_name, plugin_path, plugin_config) {
	log(util::debug, "Creating imgui GUI");
		
	this->gui_thread = new std::thread(&ImGuiGUI::event_loop, this);
}

ImGuiGUI::~ImGuiGUI() {
	if (this->gui_thread != nullptr)
		if (this->gui_thread->joinable())
			this->gui_thread->join();
		
	if (window != nullptr) {
		this->cleanup_window();
	}
}

void ImGuiGUI::event_loop() {
	// The window must be set up in the event loop since opengl contexts are
	// thread-local
	this->window = setup_imgui();

	if (this->window == nullptr) {
		util::log(util::error, "Window is null");
		return;
	}

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		
	while (!glfwWindowShouldClose(window) && !::fpsi::session->exiting) {
		// Start the Dear ImGui frame
		glfwPollEvents();  // Poll and handle events (inputs, window resize, etc.)
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Create appropriate windows
		this->session_window_event();  // Create session window
		this->data_window_event();  // Create data window
		this->state_window_event();  // Create state window
		this->about_window();
			
		// Finish rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
			
	}
	this->cleanup_window();
	::fpsi::session->finish();  // Close app if window closes
}

void ImGuiGUI::cleanup_window() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
		
	glfwDestroyWindow(window);
	glfwTerminate();

	this->window = nullptr;
}

void ImGuiGUI::session_window_event() {
	ImGui::Begin("Session");  // Set window title

	// Config TODO
	
	// Plugin helper
	auto loaded_plugins = ::fpsi::session->get_loaded_plugins();
	if (ImGui::TreeNode("Plugins")) {
		for (auto plugin : loaded_plugins) {
			if (ImGui::TreeNode(plugin->name.c_str())) {
				bool unload_is_disabled = (plugin.get() == this);
				ImGui::Text("%s", plugin->path.c_str());
				ImGui::SameLine();
				if (unload_is_disabled) ImGui::BeginDisabled();
				if (ImGui::Button("unload")) {
					::fpsi::session->unload_plugin(plugin->name);
				}
				if (unload_is_disabled) ImGui::EndDisabled();
				ImGui::TreePop();
			}
		}
			
		ImGui::TreePop();
	}

	// About menu
	if (ImGui::Button("About"))
		this->show_about_window ^= true;

	// Quit button
	if (ImGui::Button("Quit"))
		::fpsi::session->finish();
		
	// default
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		
	ImGui::End();
}

void ImGuiGUI::data_window_event() {
	ImGui::Begin("DataHandler");  // Set window title
		
	// Tabs
	if (ImGui::BeginTabBar("tabs")) {
		std::vector<std::pair<std::string, std::function<std::deque<std::shared_ptr<DataFrame>>(std::shared_ptr<DataSource>)>>>
			data_types = {
			{
				"Aggregated",
				[](std::shared_ptr<DataSource> d) {
					return d->agg_data;
				}
			},
			{
				"Raw",
				[](std::shared_ptr<DataSource> d) {
					return d->raw_data;
				}
			}
		};
		for (auto [tab_name, df_from_source] : data_types) {
			if (ImGui::BeginTabItem(tab_name.c_str())) {
				static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
				static float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
				static ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 11);
				if (ImGui::BeginTable("table_scrollx", 5, flags, outer_size)) {
					//ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);
					ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_NoHide); // Make the first column not hideable to match our use of TableSetupScrollFreeze()
					ImGui::TableSetupColumn("Source");
					ImGui::TableSetupColumn("Type");
					ImGui::TableSetupColumn("Data");
					ImGui::TableSetupColumn("Time");
					ImGui::TableHeadersRow();
					for (const auto &source_name : ::fpsi::session->data_handler->get_sources()) {
						auto data_source = ::fpsi::session->data_handler->get_source(source_name);
						for (const auto &df : df_from_source(data_source)) {
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("%ld", df->get_id());
							ImGui::TableSetColumnIndex(1);
							ImGui::Text("%s", df->get_source().c_str());
							ImGui::TableSetColumnIndex(2);
							ImGui::Text("%s", df->get_type().c_str());
							ImGui::TableSetColumnIndex(3);
							ImGui::Text("%s", df->get_data().dump().c_str());
							ImGui::TableSetColumnIndex(4);
							ImGui::Text("%s", df->get_time().c_str());
						}
					}
					ImGui::EndTable();
				}
				ImGui::EndTabItem();
			}
		}
		
		ImGui::EndTabBar();
	}
	ImGui::End();
}

void ImGuiGUI::state_window_event() {
	ImGui::Begin("States");  // Set window title

	// State creator
	

	// State table
	static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
	static float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
	static ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 8);
	if (ImGui::BeginTable("table_scrollx2", 2, flags, outer_size)) {
		ImGui::TableSetupColumn("State");
		ImGui::TableSetupColumn("Value");
		//ImGui::TableSetupColumn("Time");
		ImGui::TableHeadersRow();
		for (const auto &key : ::fpsi::session->get_state_keys()) {
			auto value = ::fpsi::session->get_state(key);
			
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s", key.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", value.dump().c_str());
		}
		ImGui::EndTable();
	}
	ImGui::End();
}

void ImGuiGUI::about_window() {
	util::log(util::error, "show_about: %d", this->show_about_window);
	if (!this->show_about_window) return;
	
	ImGui::Begin("About");

	ImGui::Text("FPSI");
	ImGui::Text("Data and state control software with dynamic plugin support for use with single or multi-node systems.");
	ImGui::Text("Version: %s", ::fpsi::version.c_str());
	ImGui::Text("Created by %s", ::fpsi::author.c_str());
	ImGui::Text("%s", ::fpsi::url.c_str());
	ImGui::Text("Under %s license", ::fpsi::license.c_str());

	ImGui::End();
}

}  // namespace fpsi

extern "C" fpsi::Plugin *construct_plugin(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config) {
  return new fpsi::ImGuiGUI(plugin_name, plugin_path, plugin_config);
}
