// usb-radio.cpp

#include "plugin/plugin.hpp"

#include <algorithm>
#include <sstream>

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

namespace util {

static std::string to_lowercase(std::string data) {
	std::transform(data.begin(), data.end(), data.begin(),
    [](unsigned char c){ return std::tolower(c); });
	return data;
}

}  // namespace util


namespace fpsi {

class USBRadio : public Plugin {
public:
  USBRadio(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config) :
		Plugin(plugin_name, plugin_path, plugin_config) {

		// Iterate devices
		SoapySDR::KwargsList results = SoapySDR::Device::enumerate();
		SoapySDR::Kwargs::iterator it;

		if (results.size() == 0) {
			util::log(util::warning, "No SDR found");
		}

		std::string search_label = util::to_lowercase(plugin_config.value<std::string>("radio-label", "invalid label"));
		for( int i = 0; i < results.size(); ++i) {
			std::stringstream detected_info;
			detected_info << "SDR device " << i;
			for( it = results[i].begin(); it != results[i].end(); ++it) {
				detected_info << " | " << it->first << "=" << it->second;
				if (it->first == "label") {
					std::string lowercase_label = util::to_lowercase(it->second);
					if (lowercase_label.find(search_label) != std::string::npos) {
						util::log(util::message, "Found radio: %s", it->second.c_str());
					}
				}
			}
			if (plugin_config.value<bool>("show-all-detected", false)) {
				util::log(util::message, detected_info.str());
			}
		}
  }

  ~USBRadio() {
		if (sdr != nullptr) {
			SoapySDR::Device::unmake(sdr);  // Deletes?
			// delete sdr?
		}
	}

  // Called by main session loop before data should be aggregated
  virtual void pre_aggregate(const std::map<std::string, std::vector<std::shared_ptr<DataFrame>>> &raw_data) {}

	// Called by main session loop  after data has been aggregated
  virtual void post_aggregate(const std::map<std::string, std::vector<std::shared_ptr<DataFrame>>> &agg_data) {}

	// Called when a state changes (from ::fpsi::session->set_state(...))
  virtual void state_change(const std::string &key, const json &last, const json &next) {}

	// Called to broadcast a message (from ::fpsi::session->broadcast(...))
	virtual void broadcast(const json &message) {
		const std::lock_guard<std::mutex> rlock(this->radio_lock);
	}

	// Called to process a received message (from ::fpsi::session->receive(...))
	virtual void receive(const json &message) {
		const std::lock_guard<std::mutex> rlock(this->radio_lock);
	}

private:
	std::mutex radio_lock;  // Prevent radio from being used at the same time
	SoapySDR::Device *sdr = nullptr;
	
};

}

extern "C" fpsi::Plugin *construct_plugin(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config) {
  return new fpsi::USBRadio(plugin_name, plugin_path, plugin_config);
}
