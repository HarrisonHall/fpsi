// template.cpp

#include "plugin/plugin.hpp"

#include "session/session.hpp"
#include "util/logging.hpp"


namespace fpsi {

class TemplatePlugin : public Plugin {
public:
  TemplatePlugin(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config) :
		Plugin(plugin_name, plugin_path, plugin_config) {
  }

  ~TemplatePlugin() {}

  // Called by main session loop before data should be aggregated
  virtual void pre_aggregate(const std::map<std::string, std::vector<std::shared_ptr<DataFrame>>> &raw_data) {}

	// Called by main session loop  after data has been aggregated
  virtual void post_aggregate(const std::map<std::string, std::vector<std::shared_ptr<DataFrame>>> &agg_data) {}

	// Called when a state changes (from ::fpsi::session->set_state(...))
  virtual void state_change(const std::string &key, const json &last, const json &next) {}

	// Called to broadcast a message (from ::fpsi::session->broadcast(...))
	virtual void broadcast(const json &message) {}

	// Called to process a received message (from ::fpsi::session->receive(...))
	virtual void receive(const json &message) {}
  
};

}

extern "C" fpsi::Plugin *construct_plugin(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config) {
  return new fpsi::TemplatePlugin(plugin_name, plugin_path, plugin_config);
}
