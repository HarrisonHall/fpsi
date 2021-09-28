/*
  fpsi.cpp
 */

#include <iostream>


#include "../include/yaml.h"

#include "fpsi.hpp"
#include "session/session.hpp"
#include "util/logging.hpp"


int main(int argc, char **argv) {
  fpsi::Session s("config.yaml", argc, argv);
  util::log(s.to_string());
  return s.loop();
}
