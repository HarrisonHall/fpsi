/*
  dataframe.hpp

  The dataframe uses metric units.
 */

#include <string>

#include "datahandler.hpp"
#include "dataframe.hpp"

#include "../util/time.hpp"
#include "../fpsi.hpp"

namespace fpsi{

DataFrame::DataFrame() {
  this->id = 0;
  this->source = "";
  this->type = "raw";
  //this->data =;
}

DataFrame::DataFrame(const DataFrame &dataframe) {
  // TODO
}

DataFrame::DataFrame(Session &session) : DataFrame() {
  this->session = &session;
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
  Session &session, uint64_t id, std::string source, std::string type, json data
  ) : DataFrame(id, source, type, data) {}

DataFrame::~DataFrame() {
  // TODO: Store self in session->datahandler(source)
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

std::vector<char> DataFrame::get_bson() const {
  // TODO - optimize
  if (data.empty()) return std::vector<char>();
  auto as_ui = json::to_bson(this->data);
  std::vector<char> rval(as_ui.size());
  memcpy(rval.data(), as_ui.data(), as_ui.size());
  return rval;
}

void DataFrame::set_bson(std::vector<char>) {
  // TODO
  return;
}

}  // namespace fpsi
