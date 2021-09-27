/*
  fpsi.cpp
 */

#include <iostream>


#include "../include/yaml.h"

#include "fpsi.hpp"
#include "session/session.hpp"


int main(int argc, char **argv) {
  fpsi::Session s("config.yaml", argc, argv);
  std::cout << s.to_string() << std::endl;
  return s.loop();
}
