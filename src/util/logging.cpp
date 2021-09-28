#include "logging.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <utility>


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
    std::cerr << level_to_name[level] << ": " << message << std::endl;
  }
  if (level == warning || level == info || level == debug) {
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
