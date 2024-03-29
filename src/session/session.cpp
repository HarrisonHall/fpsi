// Session definition

#include <cmath>
#include <dlfcn.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <queue>
#include <vector>
#include <utility>

#include "config/config.hpp"
#include "data/datahandler.hpp"
#include "data/datasource.hpp"
#include "plugin/plugin.hpp"
#include "util/logging.hpp"
#include "util/time.hpp"

#include "session.hpp"

const size_t max_state_size = 10;


namespace fpsi {

Session::Session(int argc, char **argv) {
	// Read config
	this->config = std::make_shared<Config>(argc, argv);
	// Set up data
	this->data_handler = std::make_shared<DataHandler>(
		this->config->get_aggregations_per_second());
}



Session::~Session() {
	// Datahandler will destruct itself as a shared pointer
	// Plugins are automatically killed properly as shared pointers, but we should
	// call unload anyway
	{
		auto plugins = this->get_loaded_plugins();
		for (auto plugin : plugins) {
			this->unload_plugin(plugin->name);
		}
	}
	
}

void Session::aggregate_data() {
  if (this->exiting) return;  // Don't create threads if closing
	static std::mutex aggregate_lock;
	const std::lock_guard<std::mutex> alock(aggregate_lock);
	
	if (this->aggregate_thread.joinable())
		this->aggregate_thread.join();

	static auto aggregate_function = [this]() {
		auto plugins = this->get_loaded_plugins();
		std::vector<std::thread *> threads;

		// Get raw data
		std::map<std::string, std::vector<std::shared_ptr<DataFrame>>> new_data;
		for (auto ds_name : ::fpsi::session->data_handler->get_sources()) {
			new_data[ds_name] = ::fpsi::session->data_handler->get_new_raws(ds_name);
		}
		
		// Call pre_aggregate for each plugin
		for (auto plugin : plugins) {
			threads.push_back(new std::thread(&Plugin::pre_aggregate, plugin.get(), new_data));
		}
		// Close threads
		for (auto thread : threads) {
			thread->join();
			delete thread;
		}

		// Stop tracking raw data
		if (this->config->should_dump_raw()) {
			for (auto ds_name : ::fpsi::session->data_handler->get_sources()) {
				::fpsi::session->data_handler->get_source(ds_name)->clear_raw();
			}
		}

		// Get agg data
		std::map<std::string, std::vector<std::shared_ptr<DataFrame>>> agg_data;
		for (auto ds_name : ::fpsi::session->data_handler->get_sources()) {
			agg_data[ds_name] = ::fpsi::session->data_handler->get_new_aggs(ds_name);
		}

		// Run post-aggregate hook for each plugin
		threads.clear();
		for (auto plugin : plugins) {
			threads.push_back(new std::thread(&Plugin::post_aggregate, plugin.get(), agg_data));
		}
		for (auto thread : threads) {
			thread->join();
			delete thread;
		}
	};

	// TODO - Consider in-loop to minimize active sleep
	this->aggregate_thread = std::thread(aggregate_function);
}

std::shared_ptr<Plugin> Session::get_plugin(const std::string &name) {
	const std::lock_guard<std::mutex> plock(this->plugin_lock);
	for (const auto &plugin : this->plugins) {
		if (plugin->name == name) {
			return plugin;
		}
	}
	return nullptr;
}

std::vector<std::string> Session::get_state_keys() {
	std::lock_guard<std::mutex> slock(this->state_lock);
	std::vector<std::string> keys;

	for (const auto &key_state : this->states) {
		keys.push_back(key_state.first);
	}
	
	return keys;
}

const json Session::get_state(std::string key) {
	const std::lock_guard<std::mutex> slock(this->state_lock);
  if (this->states.find(key) != this->states.end())
    return this->states[key];
  return json(json::value_t::object);;
}

void Session::set_state(const std::string key, const json &new_value) {
	if (this->exiting) return;  // Don't create threads if closing
	static std::mutex set_state_lock;  // Protect setting actual states
	static std::vector<std::pair<std::string, json>> state_queue;  // State-change queue
	static std::mutex state_queue_lock;  // Protect state-change queue
	
	// Add current values to queue
	{
		const std::lock_guard<std::mutex> sqlock(state_queue_lock);
		state_queue.push_back(std::make_pair(key, new_value));
	}

	// Function to notify each plugin of state change
	static auto change_state_function = [this](auto state_queue) {
		auto plugins = this->get_loaded_plugins();
		for (const auto &[key, next_value] : state_queue) {
			const auto last_value = this->get_state(key);  // Get last value
			{
				std::lock_guard<std::mutex> slock(this->state_lock);
				this->states[key] = next_value;  // Set to new value
			}
			// Call state_change for each plugin
			std::vector<std::thread *> threads;
			for (auto plugin : plugins) {
				threads.push_back(new std::thread(&Plugin::state_change, plugin.get(), key, last_value, next_value));
			}
			// Close threads
			for (auto thread : threads) {
				thread->join();
				delete thread;
			}
		}
		set_state_lock.unlock();  // Unlock when finished
	};

	// If state thread is not running, go ahead and run thread
	if (set_state_lock.try_lock()) {
		// Join last thread
		if (this->state_thread.joinable())
			this->state_thread.join();
		// Start update thread
		const std::lock_guard<std::mutex> sqlock(state_queue_lock);
		this->state_thread = std::thread(change_state_function, state_queue);
		state_queue.clear();
	}
}

void Session::broadcast(const json &message, bool forward) {
	if (this->exiting) return;  // Don't create threads if closing
	static std::mutex broadcast_lock;
	const std::lock_guard<std::mutex> block(broadcast_lock);
	
	if (this->broadcast_thread.joinable())
		broadcast_thread.join();
	
	json packed_message = message;
	if (!forward) {
		packed_message = {
			{"from", this->config->get_name()},  // Mark us as sender
			{"timestamp", util::timestamp()},  // Give unique message id
			{"forward", true},  // Set forwarding to true to reach all nodes
			{"message", message}  // Embed message
		};
	}

	static auto broadcast_function = [this](const json message) {
		auto plugins = this->get_loaded_plugins();
		std::vector<std::thread *> threads;
		// Call broadcast for each plugin
		for (auto plugin : plugins) {
			threads.push_back(new std::thread(&Plugin::broadcast, plugin.get(), message));
		}
		// Close threads
		for (auto thread : threads) {
			thread->join();
			delete thread;
		}
	};

	this->broadcast_thread = std::thread(broadcast_function, packed_message);
}

void Session::receive(const json &message) {
	if (this->exiting) return;  // Don't create threads if closing
	static std::mutex receive_lock;
	const std::lock_guard<std::mutex> block(receive_lock);

	// Check message for validity
	std::string sender_name = message.value<std::string>("from", "");
	size_t sender_time = message.value<size_t>("timestamp", 0);
	bool is_name_empty = (sender_name.size() == 0);
	bool is_from_self = (this->config->get_name() == sender_name);
	bool has_valid_timestamp = true;
	{
		const std::lock_guard<std::mutex> nlock(this->node_times_lock);
		if (this->node_times.find(sender_name) != this->node_times.end()) {
			has_valid_timestamp = (sender_time > this->node_times[sender_name]);
		}
	}
	if (is_name_empty || is_from_self || !has_valid_timestamp) {
		//util::log(util::warning, "Received invalid message %s", message.dump().c_str());
		//util::log(util::warning, "Received invalid message %d %d %d", is_name_empty, is_from_self, has_valid_timestamp);
		return;
	}
	
	if (this->receive_thread.joinable())
		this->receive_thread.join();

	static auto receive_function = [this](const json message) {
		auto plugins = this->get_loaded_plugins();
		std::vector<std::thread *> threads;
		// Call broadcast for each plugin
		for (auto plugin : plugins) {
			threads.push_back(new std::thread(&Plugin::receive, plugin.get(), message));
		}
		// Close threads
		for (auto thread : threads) {
			thread->join();
			delete thread;
		}
		// Broadcast back out
		static const bool forward_message = true;
		::fpsi::session->broadcast(message, forward_message);
	};
	// Mark send time into map
	{
		const std::lock_guard<std::mutex> nlock(this->node_times_lock);
		this->node_times[sender_name] = sender_time;
	}
	
	this->receive_thread = std::thread(receive_function, message);
}

std::shared_ptr<Plugin> Session::load_plugin(const std::string &plugin_name, const json &plugin_info) {
	static std::vector<void *> so_handles;  // TODO - figure out what to do with these

	// Ensure plugin actually exists
	std::string plugin_file = plugin_info.value<std::string>("path", "");
	if (plugin_file.size() == 0) {
		util::log(util::warning, "No path provided for plugin %s", plugin_name.c_str());
		return nullptr;
	}
	if (plugin_file.substr(0, 1) != std::string("/")) plugin_file = "./" + plugin_file;
  std::filesystem::path fpath(plugin_file);
  if (!std::filesystem::exists(fpath)) {
    util::log(util::warning, "File %s does not exist", plugin_file.c_str());
    return nullptr;
  }
	std::string real_plugin_location = {std::filesystem::absolute(fpath)};

	// Get handle to shared-object file
	//// Allow symbols to be resolved dynamically and allow plugins to have each other's symbols
  void *so_handle = dlopen(real_plugin_location.c_str(), RTLD_LAZY | RTLD_GLOBAL);
  if (!so_handle) {
    util::log(util::warning, "Unable to open handle to so for %s", plugin_name.c_str());
    return nullptr;
  }

	// Get construct function
  void *acquire_func = dlsym(so_handle, "construct_plugin");
  if (!acquire_func) {
    util::log(util::warning, "Unable to get function for construct_plugin fo %s", plugin_name.c_str());
    return nullptr;
  }
  Plugin *(*acq)(const std::string &, const std::string &, const json &);
  acq = (Plugin *(*)(const std::string &, const std::string &, const json &))acquire_func;

	// Load plugin
  Plugin *new_plugin = acq(plugin_name, real_plugin_location, plugin_info.value<json>("config", json(json::value_t::object)));
  if (!new_plugin) {
    util::log(util::warning, "Unable to initialize plugin object for %s", plugin_name.c_str());
    return nullptr;
  }

	// Track plugin
	{
		const std::lock_guard<std::mutex> plock(this->plugin_lock);
		this->plugins.push_back(std::shared_ptr<Plugin>(new_plugin));
		so_handles.push_back(so_handle);  // Track handle - TODO - in the future consider unloading these at unload
	}
	util::log(util::debug, "Loaded plugin for %s", plugin_name.c_str());
	return this->plugins.back();
}

bool Session::load_plugins_from_config() {
	// Get plugins from config
	std::vector<std::pair<std::string, json>> raw_plugins;
  nlohmann::ordered_json config_plugins = this->config->get_plugins();
  for (const auto &plug_iter : config_plugins.items()) {
    std::string plug_name = plug_iter.key();
    json plug_info = plug_iter.value();
    raw_plugins.push_back(std::make_pair(plug_name, plug_info));
  }
	
	// Load plugins
	for (const auto &plug_info : raw_plugins) {
		auto plugin_name = plug_info.first;
		auto plugin_conf = plug_info.second;
		auto new_plugin = this->load_plugin(plugin_name, plugin_conf);
	}
	return true;
}

bool Session::unload_plugin(const std::string &name) {
	const std::lock_guard<std::mutex> plock(this->plugin_lock);
	// Find plugin
	std::shared_ptr<Plugin> found_plugin = nullptr;
	for (const auto &plugin : this->plugins) {
		if (plugin->name == name) {
			found_plugin = plugin;
			break;
		}
	}
	if (!found_plugin) return false;

	// Get rid of plugin (or rather, stop tracking it)
	found_plugin->unload();
	this->plugins.erase(std::find(this->plugins.begin(), this->plugins.end(), found_plugin));
	// Plugin should naturally deconstruct after this return and every other
	// plugin referencing it also dies
	return true;
}

std::vector<std::shared_ptr<Plugin>> Session::get_loaded_plugins() {
	const std::lock_guard<std::mutex> plock(this->plugin_lock);
	return this->plugins;
}


/*
  Close threads.
 */
void Session::finish() {
  if (this->exiting) return;

  this->exiting = true;

	util::log(util::debug, "Killing recv");
	if (this->receive_thread.joinable())
		this->receive_thread.join();

	util::log(util::debug, "Killing broadcast");
	if (this->broadcast_thread.joinable())
		this->broadcast_thread.join();

  util::log(util::debug, "Killing agg");
	if (this->aggregate_thread.joinable())
		this->aggregate_thread.join();

  util::log(util::debug, "Killing state");
	if (this->state_thread.joinable())
		this->state_thread.join();
	
	this->data_handler->close_data_sources();  // Dump data before killing plugins

  util::log("Finished");
}

}  // namespace fpsi
