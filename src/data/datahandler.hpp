

#pragma once

#include <chrono>
#include <vector>

#include "../fpsi.hpp"
#include "dataframe.hpp"
#include "../../include/sqlite_orm.h"


namespace fpsi {

class DataHandler {

public:
  DataHandler(Session &session);
  ~DataHandler();
  void create_frame();

private:
  // eventually we want a circular buffer of data of dynamic size where data in the buffer
  // can be edited by plugins before being written to disk
  double entry_rate = 1.0;  // Number of data entries per second
};

}
