/*
  time.cpp
 */


#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>


#include "../../include/date.h"
#include "logging.hpp"

#include "time.hpp"


namespace util {

unsigned long long timestamp() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

// UTC
unsigned long long parse_time_str(std::string s) {
  std::stringstream is;
  is << s;

  std::string save;
  is >> save;
  std::istringstream in{save};
  date::sys_time<std::chrono::milliseconds> tp;
  in >> date::parse("%FT%TZ", tp);
  if (in.fail()){
    in.clear();
    in.exceptions(std::ios::failbit);
    in.str(save);
    in >> date::parse("%FT%T%Ez", tp);
  }

  return std::chrono::duration_cast<std::chrono::milliseconds>(
    tp.time_since_epoch()
  ).count();
}

std::string to_time_str(unsigned long long timestamp) {
  auto t = std::chrono::milliseconds(timestamp);
  auto tp = std::chrono::time_point<std::chrono::system_clock>(t);
  return date::format("%FT%TZ", date::floor<std::chrono::milliseconds>(tp));
}

void active_sleep(uint32_t ms) {
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = ms % 1000 * 1000000;

  while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
}

}  // namespace util
