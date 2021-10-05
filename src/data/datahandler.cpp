
#include <chrono>
#include <cstring>
#include <memory>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "../session/session.hpp"

#include "dataframe.hpp"
#include "datasource.hpp"
#include "datahandler.hpp"

#include "../util/logging.hpp"
#include "../util/time.hpp"


namespace fpsi {

DataHandler::DataHandler(Session *session) {
  this->db = new sqlite_orm::internal::storage_t<sqlite_orm::internal::table_t<fpsi::DataFrame, sqlite_orm::internal::column_t<fpsi::DataFrame, unsigned long, unsigned long (fpsi::DataFrame::*)() const, void (fpsi::DataFrame::*)(unsigned long), sqlite_orm::constraints::autoincrement_t, sqlite_orm::constraints::primary_key_t<>>, sqlite_orm::internal::column_t<fpsi::DataFrame, std::basic_string<char>, std::basic_string<char> (fpsi::DataFrame::*)() const, void (fpsi::DataFrame::*)(std::basic_string<char>)>, sqlite_orm::internal::column_t<fpsi::DataFrame, std::basic_string<char>, std::basic_string<char> (fpsi::DataFrame::*)() const, void (fpsi::DataFrame::*)(std::basic_string<char>)>, sqlite_orm::internal::column_t<fpsi::DataFrame, std::vector<char>, std::vector<char> (fpsi::DataFrame::*)() const, void (fpsi::DataFrame::*)(std::vector<char>)>, sqlite_orm::internal::column_t<fpsi::DataFrame, std::basic_string<char>, std::basic_string<char> (fpsi::DataFrame::*)() const, void (fpsi::DataFrame::*)(std::basic_string<char>)>>>(
    sql::make_storage(
    "/tmp/fpsi.sqlite",
    sql::make_table(
      "packets",
      sql::make_column("id", &DataFrame::get_id, &DataFrame::set_id, sql::autoincrement(), sql::primary_key()),
      sql::make_column("source", &DataFrame::get_source, &DataFrame::set_source),
      sql::make_column("time", &DataFrame::get_time, &DataFrame::set_time),
      sql::make_column("data", &DataFrame::set_bson, &DataFrame::get_bson),
      sql::make_column("type", &DataFrame::set_type, &DataFrame::get_type)
      )
    )
  );
  this->db->sync_schema();
  this->session = session;
}

DataHandler::~DataHandler() {
}

bool DataHandler::create_data_source(const std::string &name, const json &base_packet) {
  if (this->data_sources.find(name) != this->data_sources.end()) return false;
  std::shared_ptr<DataSource> ds(new DataSource(name));
  //DataSource *ds = new DataSource{name};
  this->data_sources[name] = ds;
  auto df = this->create_raw(name, base_packet);
  return true;
}

std::shared_ptr<DataFrame> DataHandler::create_raw(const std::string &source, const json &data) {
  util::log(util::debug, "Creating raw");
  if (this->data_sources.find(source) == this->data_sources.end())
    return std::shared_ptr<DataFrame>(nullptr);

  std::shared_ptr<DataFrame> df = std::make_shared<DataFrame>(
    this->session, -1, source, "raw", data
  );
  df->set_time(util::to_time_str(util::timestamp()));
  df->set_id(this->db->insert(*df.get()));  // push into db
  this->data_sources[source]->raw_data.push_back(df);  // track it

  // Get rid of old packets
  while (this->data_sources[source]->raw_data.size() > this->MAX_DATA_IN_MEMORY) {
    this->data_sources[source]->raw_data.pop_front();
  }

  return df;
}

std::shared_ptr<DataFrame> DataHandler::create_agg(const std::string &source, const json &data) {
  util::log(util::debug, "Creating agg");
  if (this->data_sources.find(source) == this->data_sources.end())
    return std::shared_ptr<DataFrame>(nullptr);

  std::shared_ptr<DataFrame> df = std::make_shared<DataFrame>(
    this->session, -1, source, "agg", data
  );
  df->set_time(util::to_time_str(util::timestamp()));
  df->set_id(this->db->insert(*df.get()));  // push into db
  this->data_sources[source]->agg_data.push_back(df);  // track it

  // Get rid of old packets
  while (this->data_sources[source]->agg_data.size() > this->MAX_DATA_IN_MEMORY) {
    this->data_sources[source]->agg_data.pop_front();
  }

  return df;
}

std::shared_ptr<DataFrame> DataHandler::create_stt(const std::string &source, const json &data) {
  util::log(util::debug, "Creating stt");

  std::shared_ptr<DataFrame> df = std::make_shared<DataFrame>(
    this->session, -1, source, "stt", data
  );
  df->set_time(util::to_time_str(util::timestamp()));
  df->set_id(this->db->insert(*df.get()));  // push into db

  return df;
}

std::shared_ptr<DataFrame> DataHandler::get_newest_agg(const std::string &source) {
  if (this->data_sources.find(source) != this->data_sources.end()) {
    auto ds = this->data_sources[source];
    if (ds->agg_data.size() > 0) return ds->agg_data[ds->agg_data.size() - 1];
  }
  return std::shared_ptr<DataFrame>(nullptr);
}

bool DataHandler::update_data_frame(const DataFrame &df) {
  this->db->update(df);
  return true;
}

std::vector<std::shared_ptr<DataFrame>> DataHandler::get_recent_data(const std::string &source) {
  std::vector<std::shared_ptr<DataFrame>> data;
  if (this->data_sources.find(source) == this->data_sources.end()) return data;
  auto ds = this->data_sources[source];
  if (!ds->last_raw) {
    for (auto df : ds->raw_data) data.push_back(df);
  } else {
    auto next_good_df = std::find(ds->raw_data.begin(), ds->raw_data.end(), ds->last_raw)++;
    while (next_good_df != ds->raw_data.end()) {
      data.push_back(*next_good_df);
      next_good_df++;
    }
  }

  if (!data.empty())
    ds->last_raw = data.back();

  return data;
}

std::vector<std::string> DataHandler::get_sources() {
  std::vector<std::string> sources;
  for (auto [ds_name, ds] : data_sources)
    sources.push_back(ds_name);
  return sources;
}

}  // namespace fpsi
