/*
  dataframe.hpp

  The dataframe uses metric units.
 */

#include <string>

#include "dataframe.hpp"


namespace fpsi{

DataFrame::DataFrame() {
}
DataFrame::~DataFrame() {
}
json DataFrame::to_json() {
  return {
    {"pressure",
     {
       {"altitude", altitude}
     }
    },
    {"gps",
     {
       {"lat", latitude},
       {"lon", longitude}
     }
    },
    {
      "attitude",
      {
        {"pitch", pitch},
        {"yaw", yaw},
        {"roll", roll}
        }
    },
    { "custom", custom}
  };
}
std::string DataFrame::to_string() {
  return this->to_json().dump();
}

}
