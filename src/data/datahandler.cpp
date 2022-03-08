// datahandler.cpp

#include <chrono>
#include <cstring>
#include <memory>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "fpsi.hpp"
#include "dataframe.hpp"
#include "datasource.hpp"
#include "datahandler.hpp"
#include "util/logging.hpp"
#include "util/time.hpp"


namespace fpsi {

DataHandler::DataHandler(double agg_per_sec) : agg_per_sec(agg_per_sec) {}

DataHandler::~DataHandler() {}

bool DataHandler::create_data_source(const std::string &name) {
	if (this->closed) return false;
  if (this->data_sources.find(name) != this->data_sources.end()) return false;
  std::shared_ptr<DataSource> ds(new DataSource(name));
  this->data_sources[name] = ds;
  return true;
}

void DataHandler::close_data_sources() {
	for (auto [source_name, source] : this->data_sources) {
		source->close();
	}
	this->closed = true;
}

std::shared_ptr<DataFrame> DataHandler::create_raw(const std::string &source, const json &data) {
	if (this->closed) return nullptr;
  if (this->data_sources.find(source) == this->data_sources.end())
    return std::shared_ptr<DataFrame>(nullptr);

  std::shared_ptr<DataFrame> df = std::make_shared<DataFrame>(
    -1, source, "raw", data
  );
  df->set_time(util::to_time_str(util::timestamp()));
  //df->set_id(this->db.insert(*df.get()));  // push into db  // TODO - hook?
  this->data_sources[source]->raw_data.push_back(df);  // track it
  util::log("Created raw with id %d", df->get_id());

  // Get rid of old packets
  while (this->data_sources[source]->raw_data.size() > this->MAX_DATA_IN_MEMORY) {
    this->data_sources[source]->raw_data.pop_front();
  }

  return df;
}

std::shared_ptr<DataFrame> DataHandler::create_raw(std::shared_ptr<DataFrame> df) {
	if (this->closed) return nullptr;
  if (this->data_sources.find(df->get_source()) == this->data_sources.end())
    return std::shared_ptr<DataFrame>(nullptr);

  df->set_type("raw");
  this->data_sources[df->get_source()]->raw_data.push_back(df);
  return df;
}

std::shared_ptr<DataFrame> DataHandler::create_agg(const std::string &source, const json &data) {
	if (this->closed) return nullptr;
  util::log(util::debug, "Creating agg");
  if (this->data_sources.find(source) == this->data_sources.end())
    return std::shared_ptr<DataFrame>(nullptr);

  std::shared_ptr<DataFrame> df = std::make_shared<DataFrame>(
    -1, source, "agg", data
  );
  df->set_time(util::to_time_str(util::timestamp()));
  //df->set_id(this->db.insert(*df.get()));  // push into db - TODO - hook?
  this->data_sources[source]->agg_data.push_back(df);  // track it

  // Get rid of old packets
  while (this->data_sources[source]->agg_data.size() > this->MAX_DATA_IN_MEMORY) {
    this->data_sources[source]->agg_data.pop_front();
  }

  return df;
}
std::shared_ptr<DataFrame> DataHandler::create_agg(std::shared_ptr<DataFrame> df) {
	if (this->closed) return nullptr;
  if (this->data_sources.find(df->get_source()) == this->data_sources.end())
    return std::shared_ptr<DataFrame>(nullptr);

  df->set_type("agg");
  this->data_sources[df->get_source()]->raw_data.push_back(df);
  return df;
}

std::shared_ptr<DataFrame> DataHandler::create_stt(const std::string &source, const json &data) {
	if (this->closed) return nullptr;
  std::shared_ptr<DataFrame> df = std::make_shared<DataFrame>(
    -1, source, "stt", data
  );
  df->set_time(util::to_time_str(util::timestamp()));
  //df->set_id(this->db.insert(*df.get()));  // push into db // TODO -hook?

  return df;
}
std::shared_ptr<DataFrame> DataHandler::create_stt(std::shared_ptr<DataFrame> df) {
	if (this->closed) return nullptr;
  if (this->data_sources.find(df->get_source()) == this->data_sources.end())
    return std::shared_ptr<DataFrame>(nullptr);

  df->set_type("stt");
  this->data_sources[df->get_source()]->raw_data.push_back(df);
  return df;
}

std::shared_ptr<DataFrame> DataHandler::get_newest_agg(const std::string &source) {
	if (this->closed) return nullptr;
  if (this->data_sources.find(source) != this->data_sources.end()) {
    auto ds = this->data_sources[source];
    if (ds->agg_data.size() > 0) return ds->agg_data[ds->agg_data.size() - 1];
  }
  return std::shared_ptr<DataFrame>(nullptr);
}

bool DataHandler::update_data_frame(const DataFrame &df) {
  //this->db.update(df); - TODO - hook?
  return true;
}

std::vector<std::shared_ptr<DataFrame>> DataHandler::get_recent_data(const std::string &source) {
	if (this->closed) return {};
  std::vector<std::shared_ptr<DataFrame>> data;
  if (this->data_sources.find(source) == this->data_sources.end()) return data;
  auto ds = this->data_sources[source];
  if (!ds->last_raw) {
    for (auto df : ds->raw_data) data.push_back(df);
  } else {
    auto next_good_df = std::find(ds->raw_data.begin(), ds->raw_data.end(), ds->last_raw)++;
    if (next_good_df != ds->raw_data.end()) {
      while (next_good_df != ds->raw_data.end()) {
        data.push_back(*next_good_df);
        next_good_df++;
      }
    } else {
      for (auto df : ds->raw_data) data.push_back(df);
    }
  }

  if (!data.empty())
    ds->last_raw = data.back();

  return data;
}

std::shared_ptr<DataSource> DataHandler::get_source(const std::string &source) {
	if (this->data_sources.find(source) != this->data_sources.end()) {
		return this->data_sources[source];
	}
	return nullptr;
}

std::vector<std::string> DataHandler::get_sources() {
  std::vector<std::string> sources;
  for (auto [ds_name, ds] : data_sources)
    sources.push_back(ds_name);
  return sources;
}

uint32_t DataHandler::agg_delay_ms() {
	static uint32_t agg_delay = 0;
	if (!agg_delay) {
		agg_delay = static_cast<uint32_t>(1000.0 / this->agg_per_sec);
	}
	return agg_delay;
}

}  // namespace fpsi
