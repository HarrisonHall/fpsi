
#include <chrono>
#include <cstring>
#include <memory>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "../../include/sqlite_orm.h"

#include "dataframe.hpp"
#include "datahandler.hpp"

#include "../session/session.hpp"
#include "../util/logging.hpp"
#include "../util/time.hpp"

namespace sql = sqlite_orm;


namespace fpsi {

DataHandler::DataHandler(Session &session) {
  auto storage = sql::make_storage(
    "/tmp/fpsi.sqlite",
    sql::make_table(
      "packets",
      sql::make_column("id", &DataFrame::id, sql::autoincrement(), sql::primary_key()),
      sql::make_column("source", &DataFrame::source),
      sql::make_column("time", &DataFrame::time),
      sql::make_column("data", &DataFrame::set_bson, &DataFrame::get_bson),
      sql::make_column("type", &DataFrame::type)
      )
    );
  storage.sync_schema();
  util::log(util::debug, "Created storage");
  
  DataFrame test_data(-1, "rock", "raw", {});
  
  auto inserted_id = storage.insert(test_data);
  util::log("Inserted storage with id %d", inserted_id);

  auto test_data2 = storage.get<DataFrame>(inserted_id);
  util::log("Read back data as %s", test_data2.type.c_str());
}

DataHandler::~DataHandler() {}

void DataHandler::create_frame() {
  DataFrame new_frame;
  //data.push_back(new_frame);
  // TODO
}

bool DataHandler::create_data_source(const std::string &name, json &base_packet) {
  if (this->data_sources.find(name) != this->data_sources.end()) return false;
  DataSource *ds = new DataSource{name};
  this->data_sources[name] = *ds;
  return true;
}

}
