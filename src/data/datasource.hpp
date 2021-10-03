# pragma once

#include <deque>
#include <memory>
#include <string>



namespace fpsi {

class DataFrame;

class DataSource {
public:
  DataSource(const std::string &ds_name) : name(ds_name) {}
  ~DataSource() {}
  std::string name;
  std::deque<std::shared_ptr<DataFrame>> raw_data;
  std::deque<std::shared_ptr<DataFrame>> agg_data;
  std::shared_ptr<DataFrame> last_raw = nullptr;
  //std::shared_ptr<DataFrame> raw_agg = nullptr;
};

}  // namespace fpsi
