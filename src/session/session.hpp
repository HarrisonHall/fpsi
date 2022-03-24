// FPSI Session

#pragma once

#include <string>
#include <thread>
#include <deque>
#include <memory>
#include <mutex>

#include "fpsi.hpp"


extern const size_t max_state_size;

namespace fpsi {

class DataHandler;
class Plugin;
class Config;

class Session {
public:
  explicit Session(int argc, char **argv);
  ~Session();

	std::shared_ptr<Plugin> get_plugin(const std::string &name);  // Get plugin by name
	std::vector<std::shared_ptr<Plugin>> get_loaded_plugins();  // Get all loaded plugins
	std::shared_ptr<Plugin> load_plugin(const std::string &plugin_name, const json &plugin_info);  // Load individual plugin
	bool load_plugins_from_config();  // Load all plugins from config
	bool unload_plugin(const std::string &name);  // Unload a loaded plugin

  void set_state(std::string, const json &);  // Queue set state in state-registry, nonblocking to prevent deadlock
  const json get_state(std::string);  // Get state from state-registry
	std::vector<std::string> get_state_keys();  // Get all individual state keys

	
	void broadcast(const json &message, bool forward = false);  // Send message to all connected nodes
	// TODO - create method for sending to specific node
	void receive(const json &message);  // Called by plugin to notify session (and other plugins) that a message was received

  std::shared_ptr<DataHandler> data_handler;  // Access global data
	std::shared_ptr<Config> config;  // Access global config

	void aggregate_data();  // Aggregate data into new dataobjects
  void finish();  // Close threads so fpsi can exit

  std::thread aggregate_thread;
  std::thread state_thread;
	std::thread broadcast_thread;
	std::thread receive_thread;
  bool exiting = false;  // True when fpsi is exiting and threads should stop asap
  
private:
  std::map<std::string, json> states;  // Registry of state variables
	std::mutex state_lock;  // Protect state map
  std::vector<std::shared_ptr<Plugin>> plugins;  // List of plugins running (all unique)
	std::mutex plugin_lock;  // Protect plugin vector
	std::map<std::string, size_t> node_times;  // Map nodes to their last mast time to avoid repeated messages
	std::mutex node_times_lock;  // Protect node_times map
};

}
