// Session definition

#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <queue>
#include <vector>
#include <utility>

#include "CLI11.hpp"
#include "yaml.h"

#include "plugin/plugin_handler.hpp"
#include "plugin/plugin.hpp"
#include "util/yaml_json.hpp"
#include "util/logging.hpp"
#include "util/time.hpp"


#include "data/datahandler.hpp"
#include "session.hpp"

const size_t max_state_size = 10;


namespace fpsi {

Session::Session(int argc, char **argv) {
  this->parse_cli(argc, argv);
	auto config_yaml = YAML::LoadFile(this->config_path);
	this->raw_config = util::from_yaml(config_yaml);

	this->data_handler = std::make_shared<DataHandler>(
		this->get_from_config<double>("aggregations_per_second", 4.0));
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
	// Datahandler will destruct itself as a shared pointer
	// Plugins are automatically killed properly as shared pointers
}

void Session::aggregate_data() {
  if (this->exiting) return;  // Don't create threads if closing
	static std::mutex aggregate_lock;
	const std::lock_guard<std::mutex> alock(aggregate_lock);
	
	if (this->aggregate_thread) {
    if (this->aggregate_thread->joinable()) {
      this->aggregate_thread->join();
      delete this->aggregate_thread;
    }
  }

	static auto aggregate_function = [this]() {
		auto plugins = this->get_loaded_plugins();
		std::vector<std::thread *> threads;

		// Get raw data
		std::map<std::string, std::vector<std::shared_ptr<DataFrame>>> new_data;
		for (auto ds_name : ::fpsi::session->data_handler->get_sources()) {
			new_data[ds_name] = ::fpsi::session->data_handler->get_recent_data(ds_name);
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

		// TODO - consider emptying raw data

		// Get agg data
		std::map<std::string, std::shared_ptr<DataFrame>> agg_data;
		for (auto ds_name : ::fpsi::session->data_handler->get_sources()) {
			agg_data[ds_name] = ::fpsi::session->data_handler->get_newest_agg(ds_name);
		}

		// Run post-aggregate hook for each plugin
		threads.clear();
		for (auto plugin : plugins) {
			threads.push_back(new std::thread(&Plugin::post_aggregate, plugin.get(), agg_data));
			//threads.push_back(new std::thread([plugin](){plugin->post_aggregate();}));
		}
		for (auto thread : threads) {
			thread->join();
			delete thread;
		}
	};

	this->aggregate_thread = new std::thread(aggregate_function);
}

std::string Session::get_name() {
  return raw_config.value<std::string>("name", "FPSI Default");
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

void Session::set_state(std::string key, const json &new_value) {
	if (this->exiting) return;  // Don't create threads if closing
	static std::mutex set_state_lock;
	const std::lock_guard<std::mutex> sslock(set_state_lock);

  if (state_thread) {
    if (state_thread->joinable()) {
      state_thread->join();
      delete state_thread;
    }
  }
	
  json last_value = this->get_state(key);
	{
		std::lock_guard<std::mutex> slock(this->state_lock);
		this->states[key] = new_value;
	}

	static auto change_state_function = [this](const std::string key, const json last_value, const json next_value) {
		auto plugins = this->get_loaded_plugins();
		std::vector<std::thread *> threads;
		// Call state_change for each plugin
		for (auto plugin : plugins) {
			threads.push_back(new std::thread(&Plugin::state_change, plugin.get(), key, last_value, next_value));
		}
		// Close threads
		for (auto thread : threads) {
			thread->join();
			delete thread;
		}
	};

	// TODO - make fpsi sqlite-db track/restore state
  this->state_thread = new std::thread(change_state_function, key, last_value, new_value);
}

void Session::broadcast(const json &message, bool forward) {
	if (this->exiting) return;  // Don't create threads if closing
	static std::mutex broadcast_lock;
	const std::lock_guard<std::mutex> block(broadcast_lock);
	
	if (broadcast_thread) {
    if (broadcast_thread->joinable()) {
      broadcast_thread->join();
      delete broadcast_thread;
    }
  }

	json packed_message = message;
	if (!forward) {
		packed_message = {
			{"from", this->get_name()},  // Mark us as sender
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

	this->broadcast_thread = new std::thread(broadcast_function, packed_message);
}

void Session::receive(const json &message) {
	if (this->exiting) return;  // Don't create threads if closing
	static std::mutex receive_lock;
	const std::lock_guard<std::mutex> block(receive_lock);

	// Check message for validity
	std::string sender_name = message.value<std::string>("from", "");
	size_t sender_time = message.value<size_t>("timestamp", 0);
	bool is_name_empty = (sender_name.size() == 0);
	bool is_from_self = (this->get_name() == sender_name);
	bool has_valid_timestamp = true;
	{
		const std::lock_guard<std::mutex> nlock(this->node_times_lock);
		if (this->node_times.find(sender_name) != this->node_times.end()) {
			has_valid_timestamp = (sender_time > this->node_times[sender_name]);
		}
	}
	if (is_name_empty || is_from_self || !has_valid_timestamp) {
		return;
	}
	
	if (receive_thread) {
    if (receive_thread->joinable()) {
      receive_thread->join();
      delete receive_thread;
    }
  }

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
	
	this->receive_thread = new std::thread(receive_function, message);
}


bool Session::load_plugin() {
	const std::lock_guard<std::mutex> plock(this->plugin_lock);
	// TODO - put plugin_handler::create_plugin here
	return false;
}

bool Session::load_plugins_from_config() {
	// Get plugins from config
	std::vector<std::pair<std::string, json>> raw_plugins;
  nlohmann::ordered_json config_plugins = this->raw_config.value<nlohmann::ordered_json>(
		"plugins", nlohmann::ordered_json::object());
  for (const auto &plug_iter : config_plugins.items()) {
    std::string plug_name = plug_iter.key();
    json plug_info = plug_iter.value();
    raw_plugins.push_back(std::make_pair(plug_name, plug_info));
  }

	// Load plugins
	for (const auto &plug_info : raw_plugins) {
		auto plugin_name = plug_info.first;
		auto plugin_conf = plug_info.second;
		auto new_plugin = create_plugin(plugin_name, plugin_conf);  // TODO Replace with load_plugin
    if (!new_plugin) {
      util::log(util::error, "Unable to load plugin for %s", plugin_name.c_str());
    } else {
			util::log(util::debug, "grabbing lock");
			const std::lock_guard<std::mutex> plock(this->plugin_lock);
			util::log(util::debug, "got lock");
      this->plugins.push_back(new_plugin);
      util::log(util::debug, "Loaded plugin for %s", plugin_name.c_str());
    }
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
	if (receive_thread)
    if (receive_thread->joinable())
      receive_thread->join();

	util::log(util::debug, "Killing broadcast");
	if (broadcast_thread)
    if (broadcast_thread->joinable())
      broadcast_thread->join();

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
