/*
  dataframe.hpp

  The dataframe uses metric units.
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "../session/session.hpp"
#include "../fpsi.hpp"

namespace fpsi {

class DataFrame {
public:
  DataFrame();
  DataFrame(Session &session);
  DataFrame(uint64_t id, std::string source, std::string type, json data);
  DataFrame(Session &session, uint64_t id, std::string source, std::string type, json data);
  DataFrame(const DataFrame &);
  ~DataFrame();
  json to_json();
  std::string to_string();

  uint64_t id;
  std::string source;
  std::string type;  // raw, aggregate
  json data = json(json::value_t::object);
  std::string time;
  Session *session;

  std::vector<char> get_bson() const;
  void set_bson(std::vector<char>);

private:
  
};

}  // namespace fpsi
