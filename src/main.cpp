/*
  fpsi.cpp
 */

#include <csignal>
#include <iostream>
#include <unistd.h>

#include "yaml.h"

#include "fpsi.hpp"
#include "config/config.hpp"
#include "session/session.hpp"
#include "data/datahandler.hpp"
#include "util/logging.hpp"
#include "util/time.hpp"


namespace fpsi {

std::unique_ptr<Session> session;

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

  fpsi::session = std::make_unique<fpsi::Session>(argc, argv);  // Create global session object
	fpsi::session->load_plugins_from_config();

	// Create startup message
	std::stringstream startup_message;
  util::log(util::message, "FPSI Node -> %s", fpsi::session->config->get_name().c_str());

	util::log("agg_delay: %u", ::fpsi::session->data_handler->agg_delay_ms());
  while (!fpsi::session->exiting) {
		// TODO - active_sleep_since_last doesn't seem to be working correctly at high intervals
    util::active_sleep_since_last(::fpsi::session->data_handler->agg_delay_ms());
    fpsi::session->aggregate_data();
  }

  fpsi::session->finish();

  return 0;
}
