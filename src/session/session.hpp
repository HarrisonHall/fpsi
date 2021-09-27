/*
  Session class.
 */
#pragma once


#include "../include/yaml.h"


namespace fpsi {

class Session {
public:
  explicit Session(std::string config_file, int argc, char **argv);

  int loop();

  std::string to_string();

  std::string get_name();

  double main_altitude();

  std::vector<std::pair<std::string, json>> get_plugins();

private:
  YAML::Node raw_config;
  
};

}
