#pragma once

#include <iostream>
#include <map>
#include <vector>

#include "../fpsi.hpp"

#include "../session/session.hpp"
#include "../data/dataframe.hpp"


namespace fpsi {
  
class Plugin {
public:
  Plugin(Session *session, const json &plugin_config) {}
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
  virtual void *get_gui() { return nullptr; }
  virtual void read_socket(const json &message) {}
  virtual void send_socket(const json &message) {}

private:
  std::string raw_log;
  
};

}

extern "C" fpsi::Plugin *construct_plugin(fpsi::Session *session,const json &plugin_config);
