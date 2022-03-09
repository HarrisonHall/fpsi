#include "fpsi/src/plugin/plugin.hpp"

#include <unistd.h>
#include <string>
#include <thread>

#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"


namespace fpsi {

class ImGuiGUI : public Plugin {
public:
  ImGuiGUI(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config);
  ~ImGuiGUI();

private:
  GLFWwindow *window = nullptr;
	std::thread *gui_thread = nullptr;

	bool show_about_window = false;

	void event_loop();
	void cleanup_window();
	void session_window_event();
	void data_window_event();
	void state_window_event();
	void about_window();
	
};

}  // namespace fpsi
