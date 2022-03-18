// time.cpp
// Implementations for basic time-related functions

#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>

#include "date.h"

#include "logging.hpp"
#include "time.hpp"


namespace util {

size_t timestamp() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

// UTC
size_t parse_time_str(const std::string &s) {
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

std::string to_time_str(const size_t timestamp) {
  auto t = std::chrono::milliseconds(timestamp);
  auto tp = std::chrono::time_point<std::chrono::system_clock>(t);
  return date::format("%FT%TZ", date::floor<std::chrono::milliseconds>(tp));
}

// Sleep for a given time as accurately as possible
void active_sleep(const uint32_t ms) {
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = ms % 1000 * 1000000;

  while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
}

// Sleep only the amount of time necessary in excess since last call
void active_sleep_since_last(const uint32_t ms) {
	static auto last_time = std::chrono::high_resolution_clock::now();
	auto this_time = std::chrono::high_resolution_clock::now();
	auto time_since_ms =  std::chrono::duration_cast<std::chrono::milliseconds>(this_time - last_time).count();
	if (time_since_ms < ms) {
		active_sleep(ms - time_since_ms);
	}
	last_time = this_time;
}

}  // namespace util
