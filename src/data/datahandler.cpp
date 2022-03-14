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
  this->data_sources[name] = std::make_shared<DataSource>(name);
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
  this->data_sources[source]->track_raw(df);  // track it
  util::log("Created raw with id %d", df->get_id());

  return df;
}

std::shared_ptr<DataFrame> DataHandler::create_raw(std::shared_ptr<DataFrame> df) {
	if (this->closed) return nullptr;
  if (this->data_sources.find(df->get_source()) == this->data_sources.end())
    return std::shared_ptr<DataFrame>(nullptr);

  df->set_type("raw");
	this->data_sources[df->get_source()]->track_raw(df);  // track it
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
	this->data_sources[source]->track_agg(df);  // Track it

  return df;
}
std::shared_ptr<DataFrame> DataHandler::create_agg(std::shared_ptr<DataFrame> df) {
	if (this->closed) return nullptr;
  if (this->data_sources.find(df->get_source()) == this->data_sources.end())
    return std::shared_ptr<DataFrame>(nullptr);

  df->set_type("agg");
	this->data_sources[df->get_source()]->track_agg(df);  // Track it
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
  //this->data_sources[df->get_source()]->raw_data.push_back(df);  // TODO
  return df;
}

std::vector<std::shared_ptr<DataFrame>> DataHandler::get_new_aggs(const std::string &source) {
	if (this->closed) return {};  // Don't get data is source is closed
	if (this->data_sources.find(source) == this->data_sources.end()) return {};
  std::vector<std::shared_ptr<DataFrame>> new_data;
  
  auto ds = this->data_sources[source];
	auto all_agg_data = ds->get_agg_data();
	for (const auto &df : all_agg_data) {
		if (df == ds->last_agg) {
			new_data.clear();
			continue;
		}
		new_data.push_back(df);
	}

	if (new_data.size() > 0)
		ds->last_agg = new_data.back();

	return new_data;
}

std::vector<std::shared_ptr<DataFrame>> DataHandler::get_new_raws(const std::string &source) {
	if (this->closed) return {};  // Don't get data is source is closed
	if (this->data_sources.find(source) == this->data_sources.end()) return {};
  std::vector<std::shared_ptr<DataFrame>> new_data;
  
  auto ds = this->data_sources[source];
	auto all_raw_data = ds->get_raw_data();
	for (const auto &df : all_raw_data) {
		if (df == ds->last_raw) {
			new_data.clear();
			continue;
		}
		new_data.push_back(df);
	}

	if (new_data.size() > 0)
		ds->last_raw = new_data.back();

	return new_data;
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
