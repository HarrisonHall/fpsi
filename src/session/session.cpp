/*
  Session class.
 */

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <queue>
#include <vector>
#include <utility>

#include "yaml.h"
#include "CLI11.hpp"

#include "plugin/plugin_handler.hpp"
#include "plugin/plugin.hpp"
#include "util/yaml_json.hpp"
#include "util/logging.hpp"


#include "data/datahandler.hpp"
#include "session.hpp"

const size_t max_state_size = 10;

namespace fpsi {

Session::Session(std::string config_file, int argc, char **argv) : data_handler(new DataHandler(this)) {
  auto config_yaml = YAML::LoadFile(config_file);
  raw_config = util::from_yaml(config_yaml);
  parse_cli(argc, argv);

  this->plugins = load_plugins(this, get_plugins(config_yaml));
}

void Session::parse_cli(int argc, char **argv) {
  CLI::App app{"Aviation data and state control software with dynamic plugin system."};

  app.add_flag("--gui", this->show_gui, "Show the gui");
  app.add_option("--glade", this->glade_file, "Specify alternative glade file");

  bool show_version = false;
  app.add_flag("-V,--version", show_version, "fpsi version");
  app.add_flag("-v,--verbose", this->verbose, "show more information");
  app.add_flag("-d,--debug", this->debug, "show debug information");

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    exit(app.exit(e));
  }

  if (show_version) {
    std::cout << "fpsi 0.1b" << std::endl;
    exit(0);
  }

  util::initialize_logging(this->verbose, this->debug);
}

Session::~Session() {
  /*
  for (auto plugin : this->plugins) {
    //delete plugin;
  }
  */
}

void Session::aggregate_data() {
  if (this->exiting) return;

  // Wait for last aggregation to complete
  if (this->aggregate_thread) {
    this->aggregate_thread->join();
    delete this->aggregate_thread;
  }
  this->aggregate_thread = new std::thread(fpsi::aggregate_data_threads, this, this->plugins);
}

std::string Session::to_string() {
  std::stringstream ss;
  ss << "Session: " << this->get_name();
  return ss.str();
}

std::string Session::get_name() {
  return raw_config.value<std::string>("name", "FPSI Default");
}

std::vector<std::pair<std::string, json>> Session::get_plugins(YAML::Node &config_yaml) {
  std::vector<std::pair<std::string, json>> plugins;
  YAML::Node config_plugins = config_yaml["plugins"];
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

const json Session::get_state(std::string key) {
  if (this->states.find(key) != this->states.end())
    return this->states[key];
  return json(json::value_t::object);;
}

void Session::set_state(std::string key, const json &new_value) {
  if (this->exiting) return;

  if (state_thread) {
    if (state_thread->joinable()) {
      state_thread->join();
      delete state_thread;
    }
  }
  json last_value = this->get_state(key);
  this->states[key] = new_value;
  this->state_thread = new std::thread(fpsi::change_state, this, this->plugins,
                                       key, last_value, new_value);

  // Write state to DB
  this->data_handler->create_stt(key, new_value);
}

/*
  Close threads.
 */
void Session::finish() {
  if (this->exiting) return;

  util::log("Killing gui");
  this->exiting = true;

  if (gui_thread)
    if (gui_thread->joinable())
      gui_thread->join();
  util::log("Killing agg");
  if (aggregate_thread)
    if (aggregate_thread->joinable())
      aggregate_thread->join();
  util::log("Killing state");
  if (state_thread)
    if (state_thread->joinable())
      state_thread->join();

  util::log("Finished");
}

}
