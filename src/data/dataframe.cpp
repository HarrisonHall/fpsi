/*
  dataframe.hpp

  The dataframe uses metric units.
 */

#include <string>

#include "fpsi.hpp"
#include "dataframe.hpp"
#include "datahandler.hpp"
#include "util/time.hpp"


namespace fpsi{

DataFrame::DataFrame() {
  this->id = 0;
  this->source = "";
  this->type = "raw";
}

DataFrame::DataFrame(const DataFrame &dataframe) {
  this->id = dataframe.id;
  this->source = dataframe.source;
  this->type = dataframe.type;
  this->data = dataframe.data;
  this->time = util::to_time_str(util::timestamp());
  this->data = dataframe.data;
}

DataFrame::DataFrame(
  uint64_t id, std::string source, std::string type, json data
) {
  this->id = id;
  this->source = source;
  this->type = type;
  this->data = data;
}

DataFrame::~DataFrame() {
	for (const auto &callback : this->destructor_callbacks) {
		callback(this);
	}
}

void DataFrame::add_destructor_callback(const std::function<void(DataFrame *)> &callback) {
	static std::mutex blocker;
	const std::lock_guard<std::mutex> lock(blocker);
	this->destructor_callbacks.push_back(callback);
}

json DataFrame::to_json() {
  return {
    {"id", this->id},
    {"source", this->source},
    {"type", this->type},
    {"data", this->data},
    {"time", this->time}
  };
}

std::string DataFrame::to_string() {
  return this->to_json().dump();
}

uint64_t DataFrame::get_id() const {
  return this->id;
}
void DataFrame::set_id(const uint64_t id) {
  const std::lock_guard<std::mutex> lock(this->data_lock);
  this->id = id;
}
std::string DataFrame::get_source() const {
  return this->source;
}
void DataFrame::set_source(const std::string source) {
  const std::lock_guard<std::mutex> lock(this->data_lock);
  this->source = source;
}
std::string DataFrame::get_time() const {
  return this->time;
}
void DataFrame::set_time(const std::string time) {
  const std::lock_guard<std::mutex> lock(this->data_lock);
  this->time = time;
}
std::string DataFrame::get_type() const {
  return this->type;
}
void DataFrame::set_type(const std::string type) {
  const std::lock_guard<std::mutex> lock(this->data_lock);
  this->type = type;
}
std::vector<char> DataFrame::get_bson() const {
  // TODO optimize
  std::vector<char> ret;
  std::vector<uint8_t> as_bson = json::to_bson(this->data);
  ret.resize(as_bson.size());
  memcpy(ret.data(), as_bson.data(), as_bson.size());
  return ret;
}
void DataFrame::set_bson(const std::vector<char> bson) {
  const std::lock_guard<std::mutex> lock(this->data_lock);
  if (bson.empty()) return;
  this->data = json::from_bson(bson.data(), bson.data() + bson.size());
}

json DataFrame::get_data() {
  return this->data;
}
void DataFrame::set_data(const json data) {
  const std::lock_guard<std::mutex> lock(this->data_lock);
  this->data = data;
}

}  // namespace fpsi
