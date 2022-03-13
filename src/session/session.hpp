// FPSI Session
// TODO - this interface needs to be cleaned up

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
	std::vector<std::shared_ptr<Plugin>> get_loaded_plugins();  // Get all loaded plugins
	bool load_plugin();
	bool load_plugins_from_config();
	bool unload_plugin(const std::string &name);

  void set_state(std::string, const json &);  // Set state in state-registry
  const json get_state(std::string);  // Get state from state-registry
	std::vector<std::string> get_state_keys();

	// Send message to all connected nodes
	void broadcast(const json &message, bool forward = false);
	// TODO - create method for sending to specific node
	// Called by plugin to notify session (and other plugins) that a message was received
	void receive(const json &message);
	

  std::shared_ptr<DataHandler> data_handler;

	void aggregate_data();  // Aggregate data into new dataobjects
  void finish();  // Close threads so fpsi can exit

  std::thread *aggregate_thread = nullptr;
  std::thread *state_thread = nullptr;
	std::thread *broadcast_thread = nullptr;
	std::thread *receive_thread = nullptr;
  bool exiting = false;

  
private:
	nlohmann::ordered_json raw_config;  // Parsed config file (converted from yaml)
	std::string config_path = "config.yaml";  // Path to config file
  bool verbose = false;  // Prints verbose (info & debug) information
  bool debug = false;  // Prints debug information
  std::map<std::string, json> states;  // Registry of state variables
	std::mutex state_lock;  // Protect state map
  std::vector<std::shared_ptr<Plugin>> plugins;  // List of plugins running (all unique)
	std::mutex plugin_lock;  // Protect plugin vector
	std::map<std::string, size_t> node_times;  // Map nodes to their last mast time to avoid repeated messages
	std::mutex node_times_lock;  // Protect node_times map
};

}
