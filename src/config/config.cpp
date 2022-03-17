// config.cpp

#include <filesystem>

#include "CLI11.hpp"
#include "yaml.h"

#include "util/yaml_json.hpp"

#include "config.hpp"


namespace fpsi {

Config::Config(int argc, char **argv) {
	this->parse_cli(argc, argv);
	auto config_yaml = YAML::LoadFile(this->config_path);
	this->config_map = util::from_yaml(config_yaml);
	this->parse_config_map();
}

Config::~Config() {}

void Config::parse_cli(int argc, char **argv) {
  CLI::App app{"Data and state control software with dynamic plugin system."};

  bool show_version = false;
  app.add_flag("-V,--version", show_version, "fpsi version");
  app.add_flag("-v,--verbose", this->verbose_printing, "show more information");
  app.add_flag("-d,--debug", this->debug_printing, "show debug information");
	app.add_flag("--dump-raw-after-agg", this->dump_raw_after_agg, "dump raw data immediately after used");
	app.add_option("--max-raw", this->max_raw_packets, "maximum number of raw packets tracked");
	app.add_option("--max-agg", this->max_agg_packets, "maximum number of aggregated packets tracked");
	app.add_option("--agg-per-second", this->aggregations_per_second, "aggregations per second");
	app.add_option("--config", this->config_path, "config file path");
	app.add_option("--node-name", this->node_name, "name of fpsi node");

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
		util::log(util::error, "Unable to parse flags.");
    exit(app.exit(e));
  }

  if (show_version) {
		util::log(util::raw, "fpsi version %s", ::fpsi::version.c_str());
    exit(0);
  }

	// Ensure config file exists
	// Quit if config does not exist
	std::filesystem::path config_file(this->config_path);
	if (!std::filesystem::exists(config_file)) {
		util::log(util::error, "%s does not exist", this->config_path.c_str());
		exit(1);
	}
}

void Config::parse_config_map() {
	this->verbose_printing = this->config_map.value<bool>("verbose", this->verbose_printing);
	this->debug_printing = this->config_map.value<bool>("debug", this->debug_printing);
	this->dump_raw_after_agg = this->config_map.value<bool>("dump-raw-after-agg", this->dump_raw_after_agg);
	this->max_raw_packets = this->config_map.value<size_t>("max-raw", this->max_raw_packets);
	this->max_agg_packets = this->config_map.value<size_t>("max-agg", this->max_agg_packets);
	this->aggregations_per_second = this->config_map.value<double>("aggregations-per-second", this->aggregations_per_second);
	this->node_name = this->config_map.value<std::string>("name", this->node_name);	
}

std::string Config::get_name() const {
  return this->node_name;
}

nlohmann::ordered_json Config::get_plugins() const {
	return this->config_map.value<nlohmann::ordered_json>(
		"plugins", nlohmann::ordered_json::object());
}

bool Config::verbose() const {
	return this->verbose_printing;
}

bool Config::debug() const {
	return this->debug_printing;
}

bool Config::should_dump_raw() const {
	return this->dump_raw_after_agg;
}

size_t Config::get_max_raw_packets() const {
	return this->max_raw_packets;
}

size_t Config::get_max_agg_packets() const {
	return this->max_agg_packets;
}

double Config::get_aggregations_per_second() const {
	return this->aggregations_per_second;
}


}  // namespace fpsi
