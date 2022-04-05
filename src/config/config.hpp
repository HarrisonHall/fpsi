// config.hpp

#pragma once

#include <string>

#include "fpsi.hpp"


namespace fpsi {

class Config {
public:
	Config(int argc, char **argv);
	~Config();
	
	std::string get_name() const;
	nlohmann::ordered_json get_plugins() const;

	bool verbose() const;
	bool debug() const;
	bool should_dump_raw() const;
	size_t get_max_raw_packets() const;
	size_t get_max_agg_packets() const;
	double get_aggregations_per_second() const;
	size_t get_log_buffer_size() const;

private:
	void parse_cli(int argc, char **argv);
	void parse_config_map();
	
	nlohmann::ordered_json config_map;  // Parsed config file (converted from yaml)
	
	std::string config_path = "config.yaml";  // Path to config file
	std::string node_name = "FPSI Node";
	bool verbose_printing = false;  // Prints verbose (info & debug) information
	bool debug_printing = false;  // Prints debug information
	bool dump_raw_after_agg = false;  // Dump raw packets after used
	size_t max_raw_packets = 10;  // Max number of raw packets tracked
	size_t max_agg_packets = 35;  // Max number of aggregated packets tracked
	double aggregations_per_second = 2.0;  // Number of aggregations per second
	size_t log_buffer_size = 40;  // Number of log messages kept in memory
	
};

}  // namespace fpsi
