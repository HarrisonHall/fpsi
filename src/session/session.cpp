/*
  Session class.
 */

#include <cmath>
#include <filesystem>
#include <iostream>
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

Session::Session(int argc, char **argv) {
	::fpsi::session = this;  // Session should be a global singleton
  this->parse_cli(argc, argv);
	auto config_yaml = YAML::LoadFile(this->config_path);
	raw_config = util::from_yaml(config_yaml);

	this->data_handler = std::make_shared<DataHandler>(
		this->get_from_config<double>("aggregations_per_second", 4.0));
  this->plugins = load_plugins(this->get_plugins(config_yaml));
}

void Session::parse_cli(int argc, char **argv) {
  CLI::App app{"Aviation data and state control software with dynamic plugin system."};

  bool show_version = false;
  app.add_flag("-V,--version", show_version, "fpsi version");
  app.add_flag("-v,--verbose", this->verbose, "show more information");
  app.add_flag("-d,--debug", this->debug, "show debug information");
	app.add_option("--config", this->config_path, "config file path");

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
		util::log(util::error, "Unable to parse config file.");
    exit(app.exit(e));
  }

  if (show_version) {
		util::log(util::raw, "fpsi 0.2");
    exit(0);
  }

	// Ensure config file exists
	// Quit if config does not exist
	std::filesystem::path config_file(this->config_path);
	if (!std::filesystem::exists(config_file)) {
		util::log(util::error, "%s does not exist", this->config_path.c_str());
		exit(1);
	}

  util::initialize_logging(this->verbose, this->debug);
}

Session::~Session() {
	// Plugins are automatically killed properly as shared pointers
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

std::string Session::get_name() {
  return raw_config.value<std::string>("name", "FPSI Default");
}

std::shared_ptr<Plugin> Session::get_plugin(const std::string &name) {
	for (const auto &plugin : this->plugins) {
		if (plugin->name == name) {
			return plugin;
		}
	}
	return nullptr;
}

std::vector<std::pair<std::string, json>> Session::get_plugins(YAML::Node &config_yaml) {
  std::vector<std::pair<std::string, json>> plugins;
  YAML::Node config_plugins = config_yaml["plugins"];
  for (auto plug_iter : config_plugins) {
    std::string plug_name = plug_iter.first.as<std::string>("");
    json plug_info = util::from_yaml(plug_iter.second);
    plugins.push_back(std::make_pair(plug_name, plug_info));
  }
  return plugins;
}

std::vector<std::string> Session::get_state_keys() {
	std::vector<std::string> keys;

	for (const auto &key_state : this->states) {
		keys.push_back(key_state.first);
	}
	
	return keys;
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

  this->exiting = true;

  util::log(util::debug, "Killing agg");
  if (aggregate_thread)
    if (aggregate_thread->joinable())
      aggregate_thread->join();

  util::log(util::debug, "Killing state");
  if (state_thread)
    if (state_thread->joinable())
      state_thread->join();

	this->data_handler->close_data_sources();  // Dump data before killing plugins

  util::log("Finished");
}

}
