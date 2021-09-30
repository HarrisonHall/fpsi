/*
  time.hpp
 */

#pragma once


namespace util {

unsigned long long timestamp();

// UTC
unsigned long long parse_time_str(std::string s);

std::string to_time_str(unsigned long long timestamp);

}  // namespace util
