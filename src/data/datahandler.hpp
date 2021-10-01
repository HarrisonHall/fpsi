

#pragma once

#include <map>
#include <vector>

#include "../fpsi.hpp"

#include "dataframe.hpp"
#include "datasource.hpp"

#include "../../include/sqlite_orm.h"


namespace fpsi {

class DataHandler {

public:
  DataHandler(Session &);
  ~DataHandler();
  bool create_data_source(const std::string &, json &);
  void create_frame();

private:
  // eventually we want a circular buffer of data of dynamic size where data in the buffer
  // can be edited by plugins before being written to disk
  double entry_rate = 1.0;  // Number of data entries per second
  std::map<std::string, DataSource> data_sources;
};

}
