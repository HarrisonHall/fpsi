/*
  dataframe.hpp

  The dataframe uses metric units.
 */

#include <string>

#include "../session/session.hpp"

#include "dataframe.hpp"

#include "../util/time.hpp"
#include "../fpsi.hpp"

#include "datahandler.hpp"

namespace fpsi{

DataFrame::DataFrame() {
  this->id = 0;
  this->source = "";
  this->type = "raw";
}

DataFrame::DataFrame(const DataFrame &dataframe) {
  this->id = 0;
  this->source = dataframe.source;
  this->type = dataframe.type;
  this->data = dataframe.data;
  this->time = util::to_time_str(util::timestamp());
}

DataFrame::DataFrame(Session *session) : DataFrame() {
  this->session = session;
}

DataFrame::DataFrame(
  uint64_t id, std::string source, std::string type, json data
) {
  this->id = id;
  this->source = source;
  this->type = type;
  this->data = data;
}

DataFrame::DataFrame(
  Session *session, uint64_t id, std::string source, std::string type, json data)
  : DataFrame(id, source, type, data) {
  this->session = session;
}

DataFrame::~DataFrame() {
  util::log(util::debug, "DataFrame w/ id %d just updated in db", this->id);
  this->session->data_handler->update_data_frame(*this);
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
void DataFrame::set_id(uint64_t id) {
  const std::lock_guard<std::mutex> lock(this->data_lock);
  this->id = id;
}
std::string DataFrame::get_source() const {
  return this->source;
}
void DataFrame::set_source(std::string source) {
  const std::lock_guard<std::mutex> lock(this->data_lock);
  this->source = source;
}
std::string DataFrame::get_time() const {
  return this->time;
}
void DataFrame::set_time(std::string time) {
  const std::lock_guard<std::mutex> lock(this->data_lock);
  this->time = time;
}
std::string DataFrame::get_type() const {
  return this->type;
}
void DataFrame::set_type(std::string type) {
  const std::lock_guard<std::mutex> lock(this->data_lock);
  this->type = type;
}
std::vector<char> DataFrame::get_bson() const {
  // TODO - optimize
  if (data.empty()) return std::vector<char>();
  auto as_ui = json::to_bson(this->data);
  std::vector<char> rval(as_ui.size());
  memcpy(rval.data(), as_ui.data(), as_ui.size());
  return rval;
}
void DataFrame::set_bson(std::vector<char> bson) {
  const std::lock_guard<std::mutex> lock(this->data_lock);
  if (bson.empty()) return;
  this->data = json::parse(bson.data());
}
json DataFrame::get_data() {
  return this->data;
}
void DataFrame::set_data(json data) {
  const std::lock_guard<std::mutex> lock(this->data_lock);
  this->data = data;
}

}  // namespace fpsi
