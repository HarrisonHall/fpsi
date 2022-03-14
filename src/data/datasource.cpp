// datasource.cpp

#include <deque>
#include <memory>
#include <mutex>
#include <string>

#include "data/dataframe.hpp"

#include "data/datasource.hpp"


namespace fpsi {

void DataSource::close() {
	const std::lock_guard<std::mutex> a_lock(this->agg_lock);
	const std::lock_guard<std::mutex> r_lock(this->raw_lock);
	this->raw_data.clear();
	this->agg_data.clear();
	this->last_raw = nullptr;
		this->last_agg = nullptr;
}

std::deque<std::shared_ptr<DataFrame>> DataSource::get_raw_data() {
	const std::lock_guard<std::mutex> lock(this->raw_lock);
	return this->raw_data;
}

std::deque<std::shared_ptr<DataFrame>> DataSource::get_agg_data() {
	const std::lock_guard<std::mutex> lock(this->agg_lock);
	return this->agg_data;
}

void DataSource::track_raw(std::shared_ptr<DataFrame> df) {
	const std::lock_guard<std::mutex> lock(this->raw_lock);
	this->raw_data.push_back(df);
	
	while (this->raw_data.size() > this->MAX_DATA_IN_MEMORY) {
		this->raw_data.pop_front();
	}
}

void DataSource::track_agg(std::shared_ptr<DataFrame> df) {
	const std::lock_guard<std::mutex> lock(this->agg_lock);
	this->agg_data.push_back(df);
	
	while (this->agg_data.size() > this->MAX_DATA_IN_MEMORY) {
		this->agg_data.pop_front();
	}
}

void DataSource::clear_raw() {
	const std::lock_guard<std::mutex> r_lock(this->raw_lock);
	this->raw_data.clear();
}

}  // namespace fpsi
