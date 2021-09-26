/*
  Session class.
 */
#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

#include "../include/yaml.h"

#include "plugin_handler.hpp"
#include "util/yaml_json.hpp"

#ifdef GUI
#include "gui/gui.hpp"
#endif


namespace fpsi {

class Session {
public:
  explicit Session(std::string config_file, int argc, char **argv) {
    raw_config = YAML::LoadFile(config_file);
  }

  int loop() {
    load_plugins(get_plugins());
    #ifdef GUI
    gui_build();
    #endif
    for (auto plugin : plugin_objects) {
      std::cout << "--" << plugin->get_log() << std::endl;
    }
    return 0;
  }

  std::string to_string() {
    std::stringstream ss;
    ss << "=Session=\n";
    ss << "\t" << get_name() << "\n";
    ss << "\t" << main_altitude();
    return ss.str();
  }

  std::string get_name() {
    return raw_config["name"].as<std::string>("FPSI Default");
  }

  double main_altitude() {
    return raw_config["main_altitude"].as<double>(1600.523);
  }

  std::vector<std::pair<std::string, json>> get_plugins() {
    std::vector<std::pair<std::string, json>> plugins;
    YAML::Node config_plugins = raw_config["plugins"];
    for (auto plug_iter : config_plugins) {
      std::string plug_name = plug_iter.first.as<std::string>("");
      json plug_conf = util::from_yaml(plug_iter.second);
      plugins.push_back(std::make_pair(plug_name, plug_conf));
    }
    return plugins;
  }

private:
  YAML::Node raw_config;
  
};

}
