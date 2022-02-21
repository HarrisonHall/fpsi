#pragma once

#include <map>
#include <memory>
#include <tuple>
#include <type_traits>
#include <vector>
#include <utility>

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
  bool create_data_source(const std::string &);
  std::shared_ptr<DataFrame> create_raw(const std::string &source, const json &data);
  std::shared_ptr<DataFrame> create_raw(std::shared_ptr<DataFrame> df);
  std::shared_ptr<DataFrame> create_agg(const std::string &source, const json &data);
  std::shared_ptr<DataFrame> create_agg(std::shared_ptr<DataFrame> df);
  std::shared_ptr<DataFrame> create_stt(const std::string &source, const json &data);
  std::shared_ptr<DataFrame> create_stt(std::shared_ptr<DataFrame> df);
  bool update_data_frame(const DataFrame &df);
  std::vector<std::shared_ptr<DataFrame>> get_recent_data(const std::string &source);
  std::shared_ptr<DataFrame> get_newest_agg(const std::string &source);
  std::vector<std::string> get_sources();

private:
  void load_last_session();
  
  double entry_rate = 1.0;  // Number of data aggregates per second
  std::map<std::string, std::shared_ptr<DataSource>> data_sources;
  Session *session;
  static const unsigned int MAX_DATA_IN_MEMORY = 10;

  auto db_setup() {
    return sql::make_storage(
      "/tmp/fpsi.sqlite",
	  sql::make_table(
	    "packets",
		sql::make_column("id", &DataFrame::get_id, &DataFrame::set_id,
						 sql::autoincrement(), sql::primary_key()),
		sql::make_column("source", &DataFrame::get_source,
						 &DataFrame::set_source),
		sql::make_column("time", &DataFrame::get_time,
						 &DataFrame::set_time),
		sql::make_column("data", &DataFrame::set_bson,
						 &DataFrame::get_bson),
		sql::make_column("type", &DataFrame::set_type,
						 &DataFrame::get_type)));
  }

	using dbType = decltype(db_setup());

	dbType db;
	//decltype(&DataHandler::db_setup) db;
	//auto *db;
	//auto db;

	//decltype(DataHandler::db_setup()) db;
	//using dbType = decltype(db_setup());
	//dbType db;
	//decltype(&DataHandler::db_setup) dbType;
	//using dbType = decltype(&DataHandler::db_setup);
	
	//decltype((&DataHandler::db_setup)()) db;
	
	//using dbType = decltype(std::function{&DataHandler::db_setup})::result_type;
	//dbType db;

	//decltype(std::declval<DataHandler>().db_setup()) db;
	//static auto db;
};


}  // namespace fpsi
