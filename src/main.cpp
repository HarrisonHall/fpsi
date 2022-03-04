/*
  fpsi.cpp
 */

#include <csignal>
#include <iostream>
#include <unistd.h>

#include "yaml.h"

#include "fpsi.hpp"
#include "session/session.hpp"
#include "util/logging.hpp"
#include "util/time.hpp"


namespace fpsi {

Session *session = nullptr;

// Handle ctrl+c interrupt
// On second interrupt force-quits fpsi
void interrupt_handler(int signum) {
	static bool already_called = false;

	if (!already_called) {
		util::log(util::info, "Exiting...");
		session->finish();
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
  fpsi::session = new fpsi::Session(argc, argv);
	

	// Create startup message
	std::stringstream startup_message;
  util::log(util::message, "FPSI Node: %s", fpsi::session->get_name().c_str());

  while (!fpsi::session->exiting) {
    util::active_sleep_since_last(250);  // .25 seconds
    fpsi::session->aggregate_data();
  }

  fpsi::session->finish();

  delete fpsi::session;
  return 0;
}
