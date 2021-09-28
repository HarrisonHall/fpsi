#include "logging.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <utility>

#include "../../include/rang.hpp"


static std::map<util::log_level, std::string> level_to_name = {
  {util::debug, "DBG"},
  {util::info, "INF"},
  {util::warning, "WRN"},
  {util::error, "ERR"}
};


namespace util {

bool log(log_level level, const char* message) {
  std::string file_name = "/tmp/fpsi.log";
  std::ofstream log_file(file_name, std::ofstream::out | std::ofstream::app);
  if (log_file.good()) {
    log_file << level_to_name[level] << ": " << message << std::endl;
  }
  if (level == error) {
    std::cerr << rang::bgB::red << rang::fg::yellow << level_to_name[level]
              << rang::bg::reset << rang::fg::reset
              << ": " << message << std::endl;
  } else if (level == warning) {
    std::cout << rang::bg::yellow << rang::fg::red << level_to_name[level]
              << rang::bg::reset << rang::fg::reset
              << ": " << message << std::endl;
  } else if (level == info) {
    std::cout << level_to_name[level] << ": " << message << std::endl;
  } else if (level == debug) {
    std::cout << level_to_name[level] << ": " << message << std::endl;
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

}
