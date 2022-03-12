// counter.cpp

#include "plugin/plugin.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include <unistd.h>
#include <vector>

#include "session/session.hpp"
#include "data/datahandler.hpp"
#include "util/logging.hpp"

#ifdef GUI
#include "imgui.h"
#include "implot.h"
#include "imgui-gui.hpp"
#endif


namespace fpsi {

class Counter : public Plugin {
public:
  Counter(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config) :
		Plugin(plugin_name, plugin_path, plugin_config) {
		::fpsi::session->data_handler->create_data_source("counter");
    counter_thread = new std::thread(&Counter::count, this);
    ::fpsi::session->set_state("counter_level", {{"level", 0}});

#ifdef GUI
		auto _imgui_plugin = ::fpsi::session->get_plugin("imgui-gui");
		if (_imgui_plugin) {
			ImGuiGUI* imgui_plugin = reinterpret_cast<ImGuiGUI*>(_imgui_plugin.get());

			util::log("Registering new gui display");
			imgui_plugin->register_gui([this]() {
				this->display_gui();
			});
		} else {
			util::log(util::error, "Unable to find loaded imgui-gui plugin");
		}
#endif
  }

  ~Counter() {
    this->die = true;
    if (counter_thread) counter_thread->join();
  }

  void pre_aggregate(const std::map<std::string, std::vector<std::shared_ptr<DataFrame>>> &raw_data) {
    if (raw_data.find("counter") == raw_data.end()) return;
    auto data_packets = (*raw_data.find("counter")).second;
    double avg = 0;
    for (auto df : data_packets)
      avg += df->get_data().value<double>("value", 0.0) / double(data_packets.size());
    json new_data = {
      {"value", avg}
    };
    auto agg_df = ::fpsi::session->data_handler->create_agg("counter", new_data);
  }

  void post_aggregate(const std::map<std::string, std::shared_ptr<DataFrame>> &agg_data) {
    if (agg_data.find("counter") == agg_data.end()) return;
    auto df = (*agg_data.find("counter")).second;
    double value = df->get_data().value<double>("value", 0.0);

    int level = ::fpsi::session->get_state("counter_level").value<int>("level", 0);
    if (value <= 10) {
    } else if (value >= 10 && value <= 20 && level != 1) {
      ::fpsi::session->set_state("counter_level", {{"level", 1}});
    } else if (value >= 20 && level != 2) {
      ::fpsi::session->set_state("counter_level", {{"level", 2}});
    }
  }

  void pre_state_change(const std::string &key, const json &last, const json &next) {
    util::log("counter: (pre) state changed for %s", key.c_str());
  }
  void post_state_change(const std::string &key, const json &last, const json &next) {
    util::log("counter: (post) state changed for %s", key.c_str());
  }

#ifdef GUI
	void display_gui() {
		static std::vector<double> recent_counts;
		
		//auto last_agg = ::fpsi::session->data_handler->get_newest_agg("counter");
		/*
		if (last_agg) {
			//recent_counts.push_back(last_agg->get_data().value<double>("value", 0.0) + (rand() % 10) * 0.1);
			//if (recent_counts.size() > 400) recent_counts.erase(recent_counts.begin());
			}*/
		
		
		ImGui::Begin("Counter Graphs");
		ImGui::Text("Here!");
		/*
		if (ImPlot::BeginPlot("My Plot")) {
			//ImPlot::PlotBars("Count w/ variation", (double *)recent_counts.data(), recent_counts.size());
			//ImPlot::PlotBars("My Bar Plot", bar_data, 11);
			//ImPlot::PlotLine("My Line Plot", x_data, y_data, 1000);
			

			ImPlot::EndPlot();
			}*/
		ImGui::End();

		// Counter window
		ImGui::Begin("Count");
		ImGui::Text("Count: %lu", this->value);
		ImGui::End();
		return;
	}
#endif

private:
  std::thread *counter_thread;
  bool die = false;
  size_t value = 0;
  json base_packet = {
    {"value", 0}
  };
  
  void count() {
    while (true) {
      if (this->die) return;
      value++;
      base_packet["value"] = value;
      auto df = ::fpsi::session->data_handler->create_raw("counter", base_packet);
      usleep(300000);  // .3 seconds
    }
  }
  
};

}

extern "C" fpsi::Plugin *construct_plugin(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config) {
  return new fpsi::Counter(plugin_name, plugin_path, plugin_config);
}
