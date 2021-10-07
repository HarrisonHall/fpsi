/*
  counter.cpp
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

class Counter : public Plugin {
public:
  Counter(Session *session, const json &plugin_config) : Plugin(session, plugin_config) {
    util::log(util::message, "Created filesim");
    session->data_handler->create_data_source("altitude", this->alt_packet);
    this->session = session;
    counter_thread = new std::thread(&Counter::simulate, this);
  }

  ~Counter() {
    this->die = true;
    if (counter_thread) counter_thread->join();
  }

  void pre_aggregate(const std::map<std::string, std::vector<std::shared_ptr<DataFrame>>> &raw_data) {
    if (raw_data.find("altitude") == raw_data.end()) {
      return;
    }
    auto data_packets = (*raw_data.find("altitude")).second;
    double avg = 0;
    for (auto df : data_packets) {
      avg += df->get_data().value<double>("alt", 0.0) / double(data_packets.size());
    }

    json new_data = {
      {"alt", avg}
    };
    auto agg_df = this->session->data_handler->create_agg("altitude", new_data);
  }

  void post_aggregate(const std::map<std::string, std::shared_ptr<DataFrame>> &agg_data) {
    if (agg_data.find("altitude") == agg_data.end()) return;
    auto df = (*agg_data.find("altitude")).second;
    double value = df->get_data().value<double>("alt", 0.0);
    if (df) {
      util::log(util::warning, "Got avg value %f", value);
    }
  }

private:
  Session *session;
  std::thread *counter_thread;
  bool die = false;
  size_t value = 0;
  json alt_packet = {
    {"alt", 0}
  };

  void simulate() {
    std::ifstream data_file;
    data_file.open("data/sim_irec2019.json", std::ios::out);
    if (!data_file.good()) return;
    std::string next_line;
    while (std::getline(data_file, next_line)) {
      if (this->die) return;
      json datapacket = json::parse(next_line);
      double alt = datapacket["sensors"].value<double>("alt", 0.0);
      alt_packet["alt"] = alt;
      auto df = session->data_handler->create_raw("altitude", alt_packet);
      usleep(100000);  // .1 seconds
    }
  }
};

}

extern "C" fpsi::Plugin *construct_plugin(fpsi::Session *session, const json &plugin_config) {
  return new fpsi::Counter(session, plugin_config);
}
