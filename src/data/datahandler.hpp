

#pragma once

#include <vector>

#include "../fpsi.hpp"
#include "dataframe.hpp"

namespace fpsi {

class DataHandler {

public:
  DataHandler();
  ~DataHandler();
  void create_frame();

private:
  std::vector<DataFrame> data;
  // eventually we want a circular buffer of data of dynamic size where data in the buffer
  // can be edited by plugins before being written to disk
  double entry_rate = 1.0;  // Number of data entries per second
};

}
