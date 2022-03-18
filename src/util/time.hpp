// time.hpp

#pragma once

#include <cstdint>
#include <string>


namespace util {

size_t timestamp();  // Get timestamp

size_t parse_time_str(const std::string &s);  // Get UTC timestamp

std::string to_time_str(const size_t timestamp);

void active_sleep(const uint32_t ms);
void active_sleep_since_last(const uint32_t ms);

}  // namespace util
