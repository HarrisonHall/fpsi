/*
  dataframe.hpp

  The dataframe uses metric units.
 */

#pragma once

#include <string>

#include "../fpsi.hpp"

namespace fpsi{

class DataFrame {
public:
  DataFrame();
  ~DataFrame();
  json to_json();
  std::string to_string();

private:
  double altitude = 0;
  double pitch = 0;
  double yaw = 0;
  double roll = 0;
  double latitude = 0;
  double longitude = 0;
  json custom;
  
};

}
