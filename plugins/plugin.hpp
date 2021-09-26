#pragma once

#include <iostream>

#include "../src/fpsi.hpp"


namespace fpsi {
  
class Plugin {
public:
  Plugin(const json &plugin_config) {}
  virtual ~Plugin() {}

  // State
  virtual bool armed_start(const json & armed_data) {
    return true;
  }
  virtual bool armed_end(const json & armed_data) {
    return true;
  }

  // Utility
  virtual std::string get_log() {
    std::string log_copy = raw_log;
    raw_log = "";
    return log_copy;
  }

protected:
  void log(std::string note) {
    raw_log += note;
  }

private:
  std::string raw_log;
  
};

}

extern "C" fpsi::Plugin *construct_plugin(const json &plugin_config);

extern "C" void delete_plugin(fpsi::Plugin *plug) {
  delete plug;
}
