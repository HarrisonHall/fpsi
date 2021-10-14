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


namespace fpsi {
void interupt_handler(int signum) {
  util::log("Exiting...");
  s->finish();
}
}

int main(int argc, char **argv) {
  signal(SIGINT, fpsi::interupt_handler);
  s = new fpsi::Session("config.yaml", argc, argv);
  util::log(util::message, s->to_string());

  while (!s->exiting) {
    util::active_sleep(400);  // .4 seconds
    s->aggregate_data();
  }

  s->finish();

  delete s;
  return 0;
}
