// counter.cpp

#include "fpsi/src/plugin/plugin.hpp"

#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <thread>

#include "fpsi/src/session/session.hpp"
#include "fpsi/src/data/datahandler.hpp"
#include "fpsi/src/util/logging.hpp"


namespace fpsi {

class Counter : public Plugin {
public:
  Counter(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config) :
		Plugin(plugin_name, plugin_path, plugin_config) {
    util::log(util::debug, "Created counter");
		void *s = &(::fpsi::session);
		::fpsi::session->data_handler->create_data_source("counter");
    counter_thread = new std::thread(&Counter::count, this);
    ::fpsi::session->set_state("counter_level", {{"level", 0}});
  }

  ~Counter() {
    this->die = true;
    if (counter_thread) counter_thread->join();
  }

  void pre_aggregate(const std::map<std::string, std::vector<std::shared_ptr<DataFrame>>> &raw_data) {
    if (raw_data.find("counter") == raw_data.end()) return;
    auto data_packets = (*raw_data.find("counter")).second;
    double avg = 0;
    for (auto df : data_packets)
      avg += df->get_data().value<double>("value", 0.0) / double(data_packets.size());
    json new_data = {
      {"value", avg}
    };
    auto agg_df = ::fpsi::session->data_handler->create_agg("counter", new_data);
  }

  void post_aggregate(const std::map<std::string, std::shared_ptr<DataFrame>> &agg_data) {
    if (agg_data.find("counter") == agg_data.end()) return;
    auto df = (*agg_data.find("counter")).second;
    double value = df->get_data().value<double>("value", 0.0);
    if (df) {
      util::log(util::warning, "Got avg value %f", value);
    }
    int level = ::fpsi::session->get_state("counter_level").value<int>("level", 0);
    if (value <= 10) {
    } else if (value >= 10 && value <= 20 && level != 1) {
      ::fpsi::session->set_state("counter_level", {{"level", 1}});
    } else if (value >= 20 && level != 2) {
      ::fpsi::session->set_state("counter_level", {{"level", 2}});
    }
  }

  void pre_state_change(const std::string &key, const json &last, const json &next) {
    util::log("(pre) state changed for %s", key.c_str());
  }
  void post_state_change(const std::string &key, const json &last, const json &next) {
    util::log("(post) state changed for %s", key.c_str());
  }

private:
  std::thread *counter_thread;
  bool die = false;
  size_t value = 0;
  json base_packet = {
    {"value", 0}
  };
  
  void count() {
    while (true) {
      if (this->die) return;
      value++;
      base_packet["value"] = value;
      auto df = ::fpsi::session->data_handler->create_raw("counter", base_packet);
      usleep(300000);  // .3 seconds
    }
  }
  
};

}

extern "C" fpsi::Plugin *construct_plugin(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config) {
  return new fpsi::Counter(plugin_name, plugin_path, plugin_config);
}
