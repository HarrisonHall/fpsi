/*
  state_system.cpp
 */
#include <unistd.h>

#include <cstdio>
#include <deque>
#include <fstream>
#include <iostream>
#include <thread>

#include "fpsi/src/session/session.hpp"
#include "fpsi/src/data/datahandler.hpp"
#include "fpsi/src/util/logging.hpp"

#include "fpsi/src/plugin/plugin.hpp"

namespace fpsi {

class States : public Plugin {
public:
  States(Session *session, const json &plugin_config) : Plugin(session, plugin_config) {
    util::log(util::message, "Created State System");
    session->set_state("rocket_state", {{"state", "IDLE"}});
    session->set_state("rocket_state", {{"state", "ARM"}});
    this->session = session;
  }

  ~States() {
  }

  void post_aggregate(const std::map<std::string, std::shared_ptr<DataFrame>> &agg_data) {
    std::string current_state = this->session->get_state("rocket_state").value<std::string>("state", "IDLE");
    double new_alt = agg_data.find("altitude")->second->get_data().value<double>("alt", 0.0);

    last_alts.push_back(new_alt);
    if (last_alts.size() > 10) last_alts.pop_front();
    if (last_alts.size() < 5) return;

    if (current_state == "ARM") {
      bool going_up = true;
      double last_value = last_alts[0];
      for (auto next : last_alts) {
        if (next < last_value) going_up = false;
      }
      if (going_up) {
        util::log(util::error, "UPWARDS!!!");
        session->set_state("rocket_state", {{"state", "UPWARDS"}});
      }
    }
    if (current_state == "UPWARDS") {
      bool going_up = true;
      double last_value = last_alts[0];
      for (auto next : last_alts) {
        if (next > last_value) going_up = false;
      }
      if (going_up) {
        util::log(util::error, "DOWNWARDS");
        session->set_state("rocket_state", {{"state", "DONWARDS"}});
      }
    }
  }

private:
  Session *session;
  bool die = false;
  std::deque<double> last_alts;
  json alt_packet = {
    {"alt", 0}
  };

};

}

extern "C" fpsi::Plugin *construct_plugin(fpsi::Session *session, const json &plugin_config) {
  return new fpsi::States(session, plugin_config);
}
