/*
  Session class.
 */

#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <utility>

#include "../../include/yaml.h"
#include "../../include/CLI11.hpp"

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
  parse_cli(argc, argv);
}

int Session::loop() {
  load_plugins(get_plugins());
  
#ifdef GUI
  if (this->show_gui) {
    //std::thread gui_thread(gui_build, std::ref(*this));
    //gui_thread.join();
    this->gui_thread = new std::thread(gui_build, std::ref(*this));
  }
#endif
  
  for (auto plugin : plugin_objects) {
    std::cout << "--" << plugin->get_log() << std::endl;
  }
  
#ifdef GUI
  if (this->show_gui && this->gui_thread) {
    gui_thread->join();
  }
#endif
  
  return 0;
}

void Session::parse_cli(int argc, char **argv) {
  CLI::App app{"Aviation data and state control software with dynamic plugin system."};

  app.add_flag("--gui", this->show_gui, "Show the gui");
  app.add_option("--glade", this->glade_file, "Specify alternative glade file");

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    app.exit(e);
  }
}

std::string Session::to_string() {
  std::stringstream ss;
  ss << "=Session=\n";
  ss << "\t" << this->get_name() << "\n";
  ss << "\t" << this->main_altitude();
  return ss.str();
}

std::string Session::get_name() {
  return raw_config["name"].as<std::string>("FPSI Default");
}

double Session::main_altitude() {
  return raw_config["main_altitude"].as<double>(0.0);
}

std::vector<std::pair<std::string, json>> Session::get_plugins() {
  std::vector<std::pair<std::string, json>> plugins;
  YAML::Node config_plugins = this->raw_config["plugins"];
  for (auto plug_iter : config_plugins) {
    std::string plug_name = plug_iter.first.as<std::string>("");
    json plug_conf = util::from_yaml(plug_iter.second);
    plugins.push_back(std::make_pair(plug_name, plug_conf));
  }
  return plugins;
}

std::string Session::get_glade_file() {
  return this->glade_file;
}

}
