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

  double main_altitude();

  std::vector<std::pair<std::string, json>> get_plugins();

  std::string get_glade_file();

  std::string get_state(unsigned short relative_index = 1);

  void set_state(std::string);

  std::shared_ptr<DataHandler> data_handler;

  void finish();

private:
  YAML::Node raw_config;
  bool show_gui = false;
  std::string glade_file = "";
  std::thread *gui_thread = nullptr;
  std::thread *aggregate_thread = nullptr;
  std::deque<std::string> states;
  std::vector<std::shared_ptr<Plugin>> plugins;
};

}
