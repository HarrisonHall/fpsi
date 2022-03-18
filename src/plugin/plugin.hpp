#pragma once

#include <iostream>
#include <map>
#include <vector>

#include "fpsi.hpp"

#include "session/session.hpp"
#include "data/dataframe.hpp"


namespace fpsi {
  
class Plugin {
public:
  Plugin(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config)
		: name(plugin_name), path(plugin_path), _loaded(true) {}
	
  virtual ~Plugin() {}

	// Called by main session loop before data should be aggregated
  virtual void pre_aggregate(const std::map<std::string, std::vector<std::shared_ptr<DataFrame>>> &raw_data) {}

	// Called by main session loop after data has been aggregated
  virtual void post_aggregate(const std::map<std::string, std::vector<std::shared_ptr<DataFrame>>> &agg_data) {}

	// Called when a state changes (from ::fpsi::session->set_state(...))
  virtual void state_change(const std::string &key, const json &last, const json &next) {}

	// Called to broadcast a message (from ::fpsi::session->broadcast(...))
	virtual void broadcast(const json &message) {}

	// Called to process a received message (from ::fpsi::session->receive(...))
	virtual void receive(const json &message) {}

	// Called by fpsi when the plugin is unloaded (from ::fpsi::session->unload_plugin(...))
	virtual void unload() { this->_loaded = false; }

	// Determine whether the plugin is still tracked by fpsi 
	virtual bool loaded() const { return this->_loaded; }
	

	const std::string name;  // Name of plugin (from config.yaml)
	const std::string path;  // Relative path of plugin (from config.yaml)

protected:
	bool _loaded = false;  // Whether or not the plugin is loaded
};

}

// Provide this function so that fpsi can acquire the plugin
extern "C" fpsi::Plugin *construct_plugin(const std::string &plugin_name,
																					const std::string &plugin_path,
																					const json &plugin_config);
