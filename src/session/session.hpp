/*
  Session class.
 */
#pragma once

#include <string>
#include <thread>
#include <deque>
#include <memory>

#include "../../include/yaml.h"
#include "../fpsi.hpp"


extern const size_t max_state_size;

namespace fpsi {

class DataHandler;
class Plugin;

class Session {
public:
  explicit Session(std::string config_file, int argc, char **argv);
  ~Session();
  void parse_cli(int, char**);

  void aggregate_data();

  std::string to_string();

  std::string get_name();

  std::vector<std::pair<std::string, json>> get_plugins(YAML::Node &);

  std::string get_glade_file();

  void set_state(std::string, const json &);
  const json get_state(std::string);

  std::shared_ptr<DataHandler> data_handler;

  void finish();

  std::thread *gui_thread = nullptr;
  std::thread *aggregate_thread = nullptr;
  std::thread *state_thread = nullptr;
  bool exiting = false;
  
private:
  json raw_config;
  bool show_gui = false;
  bool verbose = false;
  bool debug = false;
  std::string glade_file = "";
  std::map<std::string, json> states;
  std::vector<std::shared_ptr<Plugin>> plugins;
};

}
