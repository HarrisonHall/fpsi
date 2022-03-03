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
// Handle ctrl+c interrupt
// On second interrupt force-quits fpsi
void interrupt_handler(int signum) {
	static bool already_called = false;

	if (!already_called) {
		util::log(util::info, "Exiting...");
		s->finish();
		already_called = true;
	} else {
		util::log(util::warning, "Forcing exit.");
		exit(1);
	}
	
}
}

int main(int argc, char **argv) {
	// Set SIGINT (ctrl+c) to call interupt_handler
  signal(SIGINT, fpsi::interrupt_handler);

	// Create global session object
  s = new fpsi::Session(argc, argv);

	// Create startup message
	std::stringstream startup_message;
  startup_message << "FPSI Node: " << s->get_name();
  util::log(util::message, startup_message.str());

  while (!s->exiting) {
    util::active_sleep(400);  // .4 seconds
    s->aggregate_data();
  }

  s->finish();

  delete s;
  return 0;
}
