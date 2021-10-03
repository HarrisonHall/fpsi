/*
  fpsi.cpp
 */

#include <csignal>
#include <iostream>
#include <unistd.h>


#include "../include/yaml.h"

#include "fpsi.hpp"
#include "session/session.hpp"
#include "util/logging.hpp"
#include "util/time.hpp"


fpsi::Session *s;
bool exiting = false;


namespace fpsi {
void interupt_handler(int signum) {
  util::log("Exiting...");
  exiting = true;
}
}

int main(int argc, char **argv) {
  signal(SIGINT, fpsi::interupt_handler);
  s = new fpsi::Session("config.yaml", argc, argv);
  util::log(s->to_string());

  while (!exiting) {
    util::active_sleep(1000);  // 1 second
    s->aggregate_data();
  }

  s->finish();

  delete s;
  return 0;
}
