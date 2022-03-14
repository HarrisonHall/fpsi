# pragma once

#include <deque>
#include <memory>
#include <mutex>
#include <string>


namespace fpsi {

class DataFrame;

class DataSource {
public:
  DataSource(const std::string &ds_name) : name(ds_name) {}
  ~DataSource() {}
	void close();
	
  std::string name;
  std::shared_ptr<DataFrame> last_raw = nullptr;
	std::shared_ptr<DataFrame> last_agg = nullptr;

	std::deque<std::shared_ptr<DataFrame>> get_raw_data();

	std::deque<std::shared_ptr<DataFrame>> get_agg_data();

	void track_raw(std::shared_ptr<DataFrame> df);

	void track_agg(std::shared_ptr<DataFrame> df);

	void clear_raw();
	
	std::mutex raw_lock;
	std::mutex agg_lock;

private:
	std::deque<std::shared_ptr<DataFrame>> raw_data;
  std::deque<std::shared_ptr<DataFrame>> agg_data;
	static const size_t MAX_DATA_IN_MEMORY = 35;
};

}  // namespace fpsi
