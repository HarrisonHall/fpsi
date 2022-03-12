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
	void close() {
		const std::lock_guard<std::mutex> a_lock(this->agg_lock);
		const std::lock_guard<std::mutex> r_lock(this->raw_lock);
		this->raw_data.clear();
		this->agg_data.clear();
		this->last_raw = nullptr;
	}
  std::string name;
  std::shared_ptr<DataFrame> last_raw = nullptr;

	std::deque<std::shared_ptr<DataFrame>> get_raw_data() {
		const std::lock_guard<std::mutex> lock(this->raw_lock);
		return this->raw_data;
	}

	std::deque<std::shared_ptr<DataFrame>> get_agg_data() {
		const std::lock_guard<std::mutex> lock(this->agg_lock);
		return this->agg_data;
	}

	void track_raw(std::shared_ptr<DataFrame> df) {
		const std::lock_guard<std::mutex> lock(this->raw_lock);
		this->raw_data.push_back(df);

		while (this->raw_data.size() > this->MAX_DATA_IN_MEMORY) {
			this->raw_data.pop_front();
		}
	}

	void track_agg(std::shared_ptr<DataFrame> df) {
		const std::lock_guard<std::mutex> lock(this->agg_lock);
		this->agg_data.push_back(df);

		while (this->agg_data.size() > this->MAX_DATA_IN_MEMORY) {
			this->agg_data.pop_front();
		}
	}
	
	std::mutex raw_lock;
	std::mutex agg_lock;

private:
	std::deque<std::shared_ptr<DataFrame>> raw_data;
  std::deque<std::shared_ptr<DataFrame>> agg_data;
	static const size_t MAX_DATA_IN_MEMORY = 35;
};

}  // namespace fpsi
