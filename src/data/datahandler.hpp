

#pragma once

#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include "../fpsi.hpp"

#include "../../include/sqlite_orm.h"

namespace sql = sqlite_orm;

namespace fpsi {

class Session;
class DataFrame;
class DataSource;

class DataHandler {
public:
  DataHandler(Session *);
  ~DataHandler();
  bool create_data_source(const std::string &, const json &);
  std::shared_ptr<DataFrame> create_raw(const std::string &source, const json &data);
  std::shared_ptr<DataFrame> create_agg(const std::string &source, const json &data);
  std::shared_ptr<DataFrame> create_stt(const std::string &source, const json &data);
  bool update_data_frame(const DataFrame &df);
  std::vector<std::shared_ptr<DataFrame>> get_recent_data(const std::string &source);
  std::shared_ptr<DataFrame> get_newest_agg(const std::string &source);
  std::vector<std::string> get_sources();

private:
  double entry_rate = 1.0;  // Number of data aggregates per second
  std::map<std::string, std::shared_ptr<DataSource>> data_sources;
  sqlite_orm::internal::storage_t<sqlite_orm::internal::table_t<fpsi::DataFrame, sqlite_orm::internal::column_t<fpsi::DataFrame, unsigned long, unsigned long (fpsi::DataFrame::*)() const, void (fpsi::DataFrame::*)(unsigned long), sqlite_orm::constraints::autoincrement_t, sqlite_orm::constraints::primary_key_t<>>, sqlite_orm::internal::column_t<fpsi::DataFrame, std::basic_string<char>, std::basic_string<char> (fpsi::DataFrame::*)() const, void (fpsi::DataFrame::*)(std::basic_string<char>)>, sqlite_orm::internal::column_t<fpsi::DataFrame, std::basic_string<char>, std::basic_string<char> (fpsi::DataFrame::*)() const, void (fpsi::DataFrame::*)(std::basic_string<char>)>, sqlite_orm::internal::column_t<fpsi::DataFrame, std::vector<char>, std::vector<char> (fpsi::DataFrame::*)() const, void (fpsi::DataFrame::*)(std::vector<char>)>, sqlite_orm::internal::column_t<fpsi::DataFrame, std::basic_string<char>, std::basic_string<char> (fpsi::DataFrame::*)() const, void (fpsi::DataFrame::*)(std::basic_string<char>)>>> *db;
  Session *session;
  static const unsigned int MAX_DATA_IN_MEMORY = 10;
};

}  // namespace fpsi
