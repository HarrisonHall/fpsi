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
	void close() {
		this->raw_data.clear();
		this->agg_data.clear();
		this->last_raw = nullptr;
	}
  std::string name;
  std::deque<std::shared_ptr<DataFrame>> raw_data;
  std::deque<std::shared_ptr<DataFrame>> agg_data;
  std::shared_ptr<DataFrame> last_raw = nullptr;
};

}  // namespace fpsi
