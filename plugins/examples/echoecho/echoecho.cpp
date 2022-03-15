// echoecho.cpp

#include "plugin/plugin.hpp"

#include <memory>
#include <sstream>
#include <thread>

#include "config/config.hpp"
#include "session/session.hpp"
#include "util/logging.hpp"
#include "util/time.hpp"


namespace fpsi {

class EchoEcho : public Plugin {
public:
  EchoEcho(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config) :
		Plugin(plugin_name, plugin_path, plugin_config) {
		auto echo_loop = []() {
			while (!::fpsi::session->exiting) {
				// Take a break
				size_t sleep_time = rand() % 3;
				util::active_sleep(sleep_time * 1000);
				
				// Send message
				std::stringstream ss;
				ss << "Hello from " << ::fpsi::session->config->get_name() << "!";
				::fpsi::session->broadcast({
						{"message", ss.str()}
					});
				//util::log(util::debug, "Sent message: %s", ss.str().c_str());
			}
		};
		this->echo_thread = std::thread(echo_loop);
  }

  ~EchoEcho() {
    if (this->echo_thread.joinable()) echo_thread.join();
  }

	void receive(const json &message) {
		util::log(util::message, "Received message: %s", message.dump().c_str());
	}


private:
  std::thread echo_thread;
  
};

}

extern "C" fpsi::Plugin *construct_plugin(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config) {
  return new fpsi::EchoEcho(plugin_name, plugin_path, plugin_config);
}
