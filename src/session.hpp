/*
  Session class.
 */
#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../include/yaml.h"


namespace fpsi {

class Session {
public:
  explicit Session(std::string config_file) {
    raw_config = YAML::LoadFile(config_file);
  }

  std::string to_string() {
    std::stringstream ss;
    ss << "=Session=\n";
    ss << "\t" << get_name() << "\n";
    ss << "\t" << main_altitude();
    return ss.str();
  }

  std::string get_name() {
    return raw_config["name"].as<std::string>("FPSI Default");
  }

  double main_altitude() {
    return raw_config["main_altitude"].as<double>(1600.523);
  }

  std::vector<std::string> get_plugins() {
    return { raw_config["plugin"].as<std::string>("") };
  }

private:
  YAML::Node raw_config;
  
};

}
