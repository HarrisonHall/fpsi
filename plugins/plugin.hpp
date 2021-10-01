#pragma once

#include <iostream>
#include <vector>

#include "../src/fpsi.hpp"

#include "../src/session/session.hpp"
#include "../src/data/dataframe.hpp"


namespace fpsi {
  
class Plugin {
public:
  Plugin(Session &session, const json &plugin_config) {}
  virtual ~Plugin() {}

  virtual void pre_aggregate(std::vector<DataFrame *> &raws) {}
  virtual void post_aggregate(std::vector<DataFrame *> &raws) {}
  virtual void pre_state_change(const std::string &last, const std::string &next) {}
  virtual void post_state_change(const std::string &last, const std::string &next) {}
  virtual void send_data(std::vector<DataFrame *> &d) {}
  std::vector<DataFrame *> read_data() {
    std::vector<DataFrame *> d;
    return d;
  }
  virtual void *get_gui() { return nullptr; }

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

extern "C" fpsi::Plugin *construct_plugin(fpsi::Session &session,const json &plugin_config);
