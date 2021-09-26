#include "plugin.hpp"


namespace fpsi {
  
class DebugPlug : public Plugin {
public:
  DebugPlug(const json &plugin_config) : Plugin(plugin_config){
    log("Called DebugPlug!");
  }
  
  virtual ~DebugPlug() {
    
  }

  bool armed_start(const json &armed_data) override {
    log("DebugPlug: Armed_start");
    return true;
  }
  
};

}

extern "C" fpsi::Plugin *construct_plugin(const json &plugin_config) {
  return new fpsi::DebugPlug(plugin_config);
}
