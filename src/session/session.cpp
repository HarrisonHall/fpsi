/*
  Session class.
 */

#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <utility>

#include "../include/yaml.h"

#include "../plugin/plugin_handler.hpp"
#include "../../plugins/plugin.hpp"
#include "../util/yaml_json.hpp"

#ifdef GUI
#include "../gui/gui.hpp"
#endif

#include "session.hpp"


namespace fpsi {

Session::Session(std::string config_file, int argc, char **argv) {
  raw_config = YAML::LoadFile(config_file);
}

int Session::loop() {
  load_plugins(get_plugins());
#ifdef GUI
  std::thread gui_thread(gui_build);
#endif
  for (auto plugin : plugin_objects) {
    std::cout << "--" << plugin->get_log() << std::endl;
  }
#ifdef GUI
  gui_thread.join();
#endif
  return 0;
}

std::string Session::to_string() {
  std::stringstream ss;
  ss << "=Session=\n";
  ss << "\t" << get_name() << "\n";
  ss << "\t" << main_altitude();
  return ss.str();
}

std::string Session::get_name() {
  return raw_config["name"].as<std::string>("FPSI Default");
}

double Session::main_altitude() {
  return raw_config["main_altitude"].as<double>(1600.523);
}

std::vector<std::pair<std::string, json>> Session::get_plugins() {
  std::vector<std::pair<std::string, json>> plugins;
  YAML::Node config_plugins = raw_config["plugins"];
  for (auto plug_iter : config_plugins) {
    std::string plug_name = plug_iter.first.as<std::string>("");
    json plug_conf = util::from_yaml(plug_iter.second);
    plugins.push_back(std::make_pair(plug_name, plug_conf));
  }
  return plugins;
}

}
