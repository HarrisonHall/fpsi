#include "plugin/plugin.hpp"

#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#include "imgui.h"

namespace fpsi {

class ImGuiGUI : public Plugin {
public:
  ImGuiGUI(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config);
  ~ImGuiGUI();

	void register_gui(std::function<void(void)> additional_gui) {
		const std::lock_guard<std::mutex> glock(this->additional_gui_lock);
		this->additional_guis.push_back(additional_gui);
	}

private:
	
	std::thread *gui_thread = nullptr;
	bool show_about_window = false;

	void event_loop();
	void cleanup_window();
	void session_window_event();
	void data_window_event();
	void state_window_event();
	void about_window();

	std::mutex additional_gui_lock;
	std::vector<std::function<void(void)>> additional_guis;
	
};

}  // namespace fpsi
