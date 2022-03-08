// imgui-gui.cpp

#include "fpsi/src/plugin/plugin.hpp"

#include <cassert>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <string>
#include <thread>

#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include "fpsi/src/session/session.hpp"
#include "fpsi/src/data/datahandler.hpp"
#include "fpsi/src/data/dataframe.hpp"
#include "fpsi/src/data/datasource.hpp"
#include "fpsi/src/util/logging.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"



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

class ImGuiGUI : public Plugin {
public:
  ImGuiGUI(const std::string &plugin_name, const json &plugin_config) :
		Plugin(plugin_name, plugin_config) {
    log(util::debug, "Creating imgui GUI");
		
		this->gui_thread = new std::thread(&ImGuiGUI::event_loop, this);
  }

  ~ImGuiGUI() {
		if (this->gui_thread != nullptr)
			if (this->gui_thread->joinable())
				this->gui_thread->join();
		
		if (window != nullptr) {
			this->cleanup_window();
		}
  }

  void post_aggregate(const std::map<std::string, std::shared_ptr<DataFrame>> &agg_data) {}

	void post_state(const std::map<std::string, std::shared_ptr<DataFrame>> &agg_data) {}

	void event_loop() {
		// The window must be set up in the event loop since opengl contexts are
		// thread-local
		this->window = setup_imgui();

		if (this->window == nullptr) {
			util::log(util::error, "Window is null");
			return;
		}
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		
		while (!glfwWindowShouldClose(window) && !::fpsi::session->exiting) {
			// Poll and handle events (inputs, window resize, etc.)
			glfwPollEvents();
			
			// Start the Dear ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			
			// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
			{
				static float f = 0.0f;
				static int counter = 0;

				ImGui::Begin("FPSI Settings");  // Set window title
				
				ImGui::Text("Debug Text: %s.", "fpsi");

				if (ImGui::Button("Shutdown"))
					::fpsi::session->finish();
				
				ImGui::Checkbox("Another Window", &show_another_window);
				
				ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
				ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
				
				if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
					counter++;
				ImGui::SameLine();
				ImGui::Text("counter = %d", counter);
				
				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				ImGui::End();
			}
			
			// 3. Show another simple window.
			if (show_another_window) {
				ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
				ImGui::Text("Hello from another window!");
				if (ImGui::Button("Close Me"))
					show_another_window = false;
				ImGui::End();
			}

			// Create data table
			{
				ImGui::Begin("DataHandler");  // Set window title

				// Tabs
				if (ImGui::BeginTabBar("tabs")) {
					if (ImGui::BeginTabItem("Aggregated")) {
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
								for (const auto &df : data_source->agg_data) {
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
					if (ImGui::BeginTabItem("State")) {
						//ImGui::Text("This is the state tabe");
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
						ImGui::EndTabItem();
					}
					ImGui::EndTabBar();
				}
				ImGui::End();
			}
			
			// Rendering
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

	void cleanup_window() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		
		glfwDestroyWindow(window);
		glfwTerminate();

		this->window = nullptr;
	}

private:
  GLFWwindow *window = nullptr;
	std::thread *gui_thread = nullptr;
	
};

}

extern "C" fpsi::Plugin *construct_plugin(const std::string &plugin_name, const json &plugin_config) {
  return new fpsi::ImGuiGUI(plugin_name, plugin_config);
}
