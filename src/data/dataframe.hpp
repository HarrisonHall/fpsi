/*
  dataframe.hpp

  The dataframe uses metric units.
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>

#include "fpsi.hpp"


namespace fpsi {

class Session;


class DataFrame {
public:
  DataFrame();
  DataFrame(Session *session);
  DataFrame(uint64_t id, std::string source, std::string type, json data);
  DataFrame(Session *session, uint64_t id, std::string source, std::string type, json data);
  DataFrame(const DataFrame &);
  ~DataFrame();
  json to_json();
  std::string to_string();

  uint64_t get_id() const;
  void set_id(uint64_t);
  std::string get_source() const;
  void set_source(std::string);
  std::string get_time() const;
  void set_time(std::string);
  std::string get_type() const;
  void set_type(std::string);
  std::vector<char> get_bson() const;
  void set_bson(std::vector<char>);

  json get_data();
  void set_data(json data);

private:
  uint64_t id;
  std::string source;
  std::string type;  // raw, agg
  json data = json(json::value_t::object);
  std::string time;
  Session *session = nullptr;
  std::mutex data_lock;
};

}  // namespace fpsi
