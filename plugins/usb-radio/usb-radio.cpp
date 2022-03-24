// usb-radio.cpp

#include "plugin/plugin.hpp"

#if __has_include("SoapySDR/Device.hpp")
#include "SoapySDR/Device.hpp"
#include "SoapySDR/Types.hpp"
#include "SoapySDR/Formats.hpp"
#elif __has_include(<SoapySDR/Device.hpp>)
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Formats.hpp>
#else
#error Soapy Not installed.
#endif


#include "session/session.hpp"
#include "util/logging.hpp"


namespace fpsi {

class USBRadio : public Plugin {
public:
  USBRadio(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config) :
		Plugin(plugin_name, plugin_path, plugin_config) {

		// Iterate devices
		SoapySDR::KwargsList results = SoapySDR::Device::enumerate();
		SoapySDR::Kwargs::iterator it;

		if (results.size() == 0) {
			util::log("No SDR found");
		}

		for( int i = 0; i < results.size(); ++i) {
			util::log("Found device #%d: ", i);
			for( it = results[i].begin(); it != results[i].end(); ++it)
				{
					util::log("%s = %s\n", it->first.c_str(), it->second.c_str());
				}
			util::log("\n");
		}
  }

  ~USBRadio() {}

  // Called by main session loop before data should be aggregated
  virtual void pre_aggregate(const std::map<std::string, std::vector<std::shared_ptr<DataFrame>>> &raw_data) {}

	// Called by main session loop  after data has been aggregated
  virtual void post_aggregate(const std::map<std::string, std::vector<std::shared_ptr<DataFrame>>> &agg_data) {}

	// Called when a state changes (from ::fpsi::session->set_state(...))
  virtual void state_change(const std::string &key, const json &last, const json &next) {}

	// Called to broadcast a message (from ::fpsi::session->broadcast(...))
	virtual void broadcast(const json &message) {}

	// Called to process a received message (from ::fpsi::session->receive(...))
	virtual void receive(const json &message) {}
  
};

}

extern "C" fpsi::Plugin *construct_plugin(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config) {
  return new fpsi::USBRadio(plugin_name, plugin_path, plugin_config);
}
