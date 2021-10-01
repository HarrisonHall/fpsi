# pragma once

#include <vector>

#include "dataframe.hpp"


namespace fpsi {

struct DataSource {
  std::string name;
  std::vector<DataFrame *> raw_data;
  std::vector<DataFrame *> agg_data;
  DataFrame *last_raw = nullptr;
  DataFrame *raw_agg = nullptr;
};

}  // namespace fpsi
