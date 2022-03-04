/*
  time.hpp
 */

#pragma once

#include <cstdint>


namespace util {

unsigned long long timestamp();

// UTC
unsigned long long parse_time_str(std::string s);

std::string to_time_str(unsigned long long timestamp);

void active_sleep(uint32_t ms);
void active_sleep_since_last(uint32_t ms);

}  // namespace util
