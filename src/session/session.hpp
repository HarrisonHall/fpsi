/*
  Session class.
 */
#pragma once

#include <string>
#include <thread>
#include <deque>
#include <memory>
#include <mutex>

#include "yaml.h"

#include "fpsi.hpp"


extern const size_t max_state_size;

namespace fpsi {

class DataHandler;
class Plugin;

class Session {
public:
  explicit Session(int argc, char **argv);
  ~Session();
	
  void parse_cli(int, char**);  // Parse CLI flags

  std::string get_name();  // Get name of current fpsi node (from config)
	template<typename T>
	T get_from_config(const std::string &key, T default_value) {
		return this->raw_config.value<T>(key, default_value);
	}

	std::shared_ptr<Plugin> get_plugin(const std::string &name);  // Get plugin by name
  std::vector<std::pair<std::string, json>> get_plugins(YAML::Node &);  // Parse plugin information from yaml
	std::vector<std::shared_ptr<Plugin>> get_loaded_plugins();  // Get all loaded plugins
	bool load_plugin();
	bool unload_plugin(const std::string &name);

  void set_state(std::string, const json &);  // Set state in state-registry
  const json get_state(std::string);  // Get state from state-registry
	std::vector<std::string> get_state_keys();

	

  std::shared_ptr<DataHandler> data_handler;

	void aggregate_data();  // Aggregate data into new dataobjects
  void finish();  // Close threads so fpsi can exit

  std::thread *aggregate_thread = nullptr;
  std::thread *state_thread = nullptr;
  bool exiting = false;

  
private:
  json raw_config;  // Parsed config file (converted from yaml)
	std::string config_path = "config.yaml";  // Path to config file
  bool verbose = false;  // Prints verbose (info & debug) information
  bool debug = false;  // Prints debug information
  std::map<std::string, json> states;  // Registry of state variables
  std::vector<std::shared_ptr<Plugin>> plugins;  // List of plugins running (all unique)
	std::mutex plugin_lock;
};

}
