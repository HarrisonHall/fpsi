#include "plugin.hpp"


namespace fpsi {
  
class DebugPlug : public Plugin {
public:
  DebugPlug(Session &session, const json &plugin_config) : Plugin(session, plugin_config) {
    log("Called DebugPlug!");
  }
  
  virtual ~DebugPlug() {
    
  }
};

}

extern "C" fpsi::Plugin *construct_plugin(fpsi::Session &session, const json &plugin_config) {
  return new fpsi::DebugPlug(session, plugin_config);
}
