/*
  logging.hpp
 */

#pragma once

#include <cstdio>
#include <string>
#include <sstream>
#include <utility>

#include "../session/session.hpp"

namespace util {

enum log_level { debug, info, warning, error };

bool log(log_level, const char*);

bool log(log_level, const std::string&);
template <typename ...Params>
bool log(log_level level, const char* message, Params &&... params) {
  bool success = false;
  int buf_size = snprintf(nullptr, 0, message, params...) + 2;
  char *expanded_message = new char[buf_size]();
  int bytes_written = snprintf(expanded_message, buf_size, message, params...);
  if (bytes_written > 0 && bytes_written < buf_size) {
    success = log(level, expanded_message);
  }
  delete[] expanded_message;
  return success;
}
bool log(log_level, const std::stringstream&);
bool log(const std::string&);
template <typename ...Params>
bool log(const char* message, Params &&... params) {
  return log(info, message, std::forward<Params>(params)...);
}
bool log(const std::stringstream&);

//bool initialized_log = false;
//bool initialize_log(const fpsi::Session &session);

}










