#pragma once

#include <iostream>
#include <string>

#include "../../include/json.hpp"
#include "../../include/yaml.h"

using nlohmann::json;

namespace util {

json from_yaml(const YAML::Node &orig) {
  YAML::Emitter emitter;
  emitter << YAML::DoubleQuoted << YAML::Flow << YAML::BeginSeq << orig;
  std::string yaml_str(emitter.c_str() + 1);
  return json::parse(yaml_str);
}

YAML::Node from_json(const json &orig) {
  return YAML::Node();  // TODO
}

}
