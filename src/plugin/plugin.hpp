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
		: name(plugin_name), path(plugin_path) {}
  virtual ~Plugin() {}

  virtual void pre_aggregate(const std::map<std::string, std::vector<std::shared_ptr<DataFrame>>> &raw_data) {}
  virtual void post_aggregate(const std::map<std::string, std::shared_ptr<DataFrame>> &agg_data) {}
  virtual void pre_state_change(const std::string &key, const json &last, const json &next) {}
  virtual void post_state_change(const std::string &key, const json &last, const json &next) {}
	
  virtual void send_data(std::vector<DataFrame *> &d) {}
  std::vector<DataFrame *> read_data() {
    std::vector<DataFrame *> d;
    return d;
  }

	const std::string name;
	const std::string path;
};

}

// Provide this function so that fpsi can acquire the plugin
extern "C" fpsi::Plugin *construct_plugin(const std::string &plugin_name,
																					const std::string &plugin_path,
																					const json &plugin_config);
