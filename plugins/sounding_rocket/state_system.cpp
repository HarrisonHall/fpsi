/*
  state_system.cpp
 */

#include "fpsi/src/plugin/plugin.hpp"

#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <thread>
#include <fstream>

#include "fpsi/src/session/session.hpp"
#include "fpsi/src/data/datahandler.hpp"
#include "fpsi/src/util/logging.hpp"


namespace fpsi {

class States : public Plugin {
public:
  States(Session *session, const json &plugin_config) : Plugin(session, plugin_config) {
    util::log(util::message, "Created State System");
    session->set_state("rocket_state", {{"state", "IDLE"}});
    this->session = session;
  }

  ~States() {
  }

  void post_aggregate(const std::map<std::string, std::shared_ptr<DataFrame>> &agg_data) {
    std::string current_state = this->session->get_state("rocket_state").value<std::string>("state", "IDLE");
    util::log(util::warning, "Current state is %s", current_state.c_str());
  }

private:
  Session *session;
  bool die = false;
  json alt_packet = {
    {"alt", 0}
  };

};

}

extern "C" fpsi::Plugin *construct_plugin(fpsi::Session *session, const json &plugin_config) {
  return new fpsi::States(session, plugin_config);
}
