// logging.cpp

#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <utility>

#include "rang.hpp"

#include "session/session.hpp"
#include "config/config.hpp"

#include "logging.hpp"


static std::map<util::log_level, std::string> level_to_name = {
  {util::debug, "DBG"},
  {util::info, "INF"},
  {util::message, "MSG"},
  {util::warning, "WRN"},
  {util::error, "ERR"}
};


namespace util {

std::queue<std::string> log_buffer;

bool log(log_level level, const char* message) {
  std::string file_name = "/tmp/fpsi.log";
  std::ofstream log_file(file_name, std::ofstream::out | std::ofstream::app);
  if (log_file.good()) {
    log_file << level_to_name[level] << ": " << message << std::endl;
  }
  if (level == util::error) {
    std::cerr << rang::bgB::red << rang::fg::yellow << level_to_name[level]
              << rang::bg::reset << rang::fg::reset
              << ": " << message << std::endl;
  } else if (level == util::warning) {
    std::cout << rang::bg::yellow << rang::fg::red << level_to_name[level]
              << rang::bg::reset << rang::fg::reset
              << ": " << message << std::endl;
  } else if (level == util::message) {
    std::cout << rang::fg::cyan << level_to_name[level]
              << rang::bg::reset << rang::fg::reset
              << ": " << message << std::endl;
  } else if (level == util::info && (::fpsi::session->config->verbose() || ::fpsi::session->config->debug())) {
    std::cout << rang::fg::blue << level_to_name[level]
              << rang::fg::reset
              << ": " << message << std::endl;
  } else if (level == util::debug && ::fpsi::session->config->debug()) {
    std::cout << rang::fg::green << level_to_name[level]
              << rang::fg::reset
              << ": " << message << std::endl;
  }

	if (level == util::raw) {
		std::cout << message << std::endl;
	}
	
  if (log_file.good()) {
    log_file.close();
    return true;
  }
  return false;
}

bool log(log_level level, const std::string &message) {
  return log(level, message.c_str());
}

bool log(log_level level, const std::stringstream &ss) {
  return log(level, ss.str());
}

bool log(const std::string &message) {
  return log(info, message);
}

bool log(const std::stringstream &ss) {
  return log(info, ss.str());
}


/*
// LogBuffer work-in-progress
LogBuffer::LogBuffer() {}

void LogBuffer::add_log(const std::string &log) {
	std::lock_guard<std::mutex>(this->log_lock);
	this->logs.push(log);
	while (this->logs.size() > ::fpsi::session->config->get_log_buffer_size()) {
		this->logs.pop();
	}
}

std::pair<std::queue<std::string>&, std::lock_guard<std::mutex>&> LogBuffer::get_logs() {
	return std::make_pair(
		std::ref(std::lock_guard<std::mutex>(this->log_lock)),
		std::ref(this->logs)
	);
	}*/

}  // namespace util
