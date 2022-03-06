// sqlite-db.cpp

#include "fpsi/src/plugin/plugin.hpp"

#include <cassert>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <string>
#include <thread>

#include "sqlite3.h"

#include "fpsi/src/session/session.hpp"
#include "fpsi/src/data/datahandler.hpp"
#include "fpsi/src/util/logging.hpp"


namespace fpsi {

class SQLiteDB : public Plugin {
public:
  SQLiteDB(const std::string &plugin_name, const json &plugin_config) :
		Plugin(plugin_name, plugin_config) {
    log(util::debug, "Creating sqlite-db");

		// Read config
		std::string db_path = plugin_config.value<std::string>("db_path", "/tmp/fpsi.sqlite");
		size_t max_recent_data = plugin_config.value<size_t>("max_recent_data", 10);

		// Set up database
		sqlite3_stmt *statement;
		int failed = 0;
		failed = sqlite3_open(db_path.c_str(), &this->db);
		if (failed) {
			this->db = nullptr;
			return;
		}

		// Ensure dataframes table exists
		std::string check_table = "SELECT name FROM sqlite_master WHERE name ='dataframes' and type='table'";
		bool table_exists = false;
		int row_cursor = 0;
		failed = sqlite3_prepare_v2(this->db, check_table.c_str(), check_table.size(), &statement, NULL);
		row_cursor = sqlite3_step(statement);
		while (row_cursor == SQLITE_ROW) {
			const char *name = reinterpret_cast<const char *>(sqlite3_column_text(statement, 0));
			table_exists = !strncmp(name, "dataframes", 15);
			row_cursor = sqlite3_step(statement);
		}
		sqlite3_finalize(statement);

		if (!table_exists) {
			std::string create_table = 
				"CREATE TABLE dataframes"
				"(df_id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"df_source TEXT, df_type TEXT, df_data BLOB, df_time TEXT)";
			failed = sqlite3_exec(this->db, create_table.c_str(), NULL, NULL, NULL);
			util::log(util::debug, "Created table dataframes");
		} else {
			util::log(util::debug, "dataframes table exists");
		}

		// Read intial data
		std::string get_data =
			"SELECT * FROM "
			"(SELECT * FROM dataframes ORDER BY df_id DESC LIMIT " + std::to_string(max_recent_data) + " )"
			"ORDER BY df_id ASC";
		failed = sqlite3_prepare_v2(this->db, get_data.c_str(), get_data.size(), &statement, NULL);
		row_cursor = sqlite3_step(statement);
		while (row_cursor == SQLITE_ROW) {
			size_t id = sqlite3_column_int(statement, 0);
			std::string source(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
			std::string type(reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)));
			const void *bson_data = sqlite3_column_blob(statement, 3);
			int bson_len = sqlite3_column_bytes(statement, 3);
			std::vector<std::uint8_t> v_bson(bson_len+1);
			memcpy(v_bson.data(), bson_data, bson_len);
			for (size_t i = 0; i < v_bson.size(); i++) {
				std::cout << "START: " << (uint16_t)v_bson.data()[i] << std::endl;
			}
			util::log("%p %d", bson_data, bson_len);
			json df_json = json::from_bson(v_bson);
			std::string time(reinterpret_cast<const char*>(sqlite3_column_text(statement, 4)));

			util::log(util::debug, "%s: %s: %s", source.c_str(), type.c_str(), time.c_str());

			::fpsi::session->data_handler->create_data_source(source);

			auto df = std::make_shared<DataFrame>(id, source, type, df_json);
			//df->set_time(time);  // TODO

			if (type == "raw") {
				::fpsi::session->data_handler->create_raw(df);
			} else if (type == "agg") {
				::fpsi::session->data_handler->create_agg(df);
			} else if (type == "state") {
				::fpsi::session->data_handler->create_stt(df);
			} else {
				util::log(util::warning, "Invalid dataframe type:  %s", type.c_str());
			}

			row_cursor = sqlite3_step(statement);
		}
		sqlite3_finalize(statement);
		
  }

  ~SQLiteDB() {
		if (this->db) {
			util::log("Closing sqlitedb");
			sqlite3_close(db);
		}
  }

  void post_aggregate(const std::map<std::string, std::shared_ptr<DataFrame>> &agg_data) {
		static auto update_in_db = [this](DataFrame *df) {
			/*
			util::log(util::debug, "Calling update callback");
			if (this->db) {
				int failed = sqlite3_exec(db, "INSERT INTO dataframes(df_source, df_type, df_data, df_time) VALUES(?, ?, ?, ?);", "test", "test1", NULL, "test2");
				if (!failed) {
				  df->set_id(sqlite3_last_insert_rowid(db));
				} else {
					util::log(util::warning, "Failed to enter df into db");
				}
			}
			return;*/
		};
		
		for (auto [source, df] : agg_data) {
			util::log(util::debug, "Inserting");
			// Insert into db
			std::string insert_df =
				"INSERT INTO dataframes(df_source, df_type, df_data, df_time) "
				"VALUES(?, ?, ?, ?)";
			sqlite3_stmt *statement;
			int failed = sqlite3_prepare_v2(this->db, insert_df.c_str(), insert_df.size(), &statement, NULL);
			
			// Bind vars
			failed = sqlite3_bind_text(statement, 1, df->get_source().c_str(), -1, SQLITE_TRANSIENT);
			assert(("Failed to bind source", failed == SQLITE_OK));
			failed = sqlite3_bind_text(statement, 2, df->get_type().c_str(), -1, SQLITE_TRANSIENT);
			assert(("Failed to bind text", failed == SQLITE_OK));
			util::log("bson: %c %d", df->get_bson().data()[0],df->get_bson().size());
			auto df_bson = df->get_bson();
			for (size_t i = 0; i < df_bson.size(); i++) {
				std::cout << "HERE: " << (uint16_t)df_bson.data()[i] << std::endl;
			}
			failed = sqlite3_bind_blob(statement, 3, df_bson.data(), df_bson.size(), SQLITE_TRANSIENT);
			assert(("Failed to bind blob", failed == SQLITE_OK));
			failed = sqlite3_bind_text(statement, 4, df->get_time().c_str(), -1, SQLITE_TRANSIENT);
			assert(("Failed to bind time", failed == SQLITE_OK));
			
			// Exec
			failed = sqlite3_step(statement);
			assert(("sqlite insert is a single operation", failed = SQLITE_DONE));
			sqlite3_finalize(statement);

			// Update id
			df->set_id(sqlite3_last_insert_rowid(db));

			// Add callback to update on deletion
			df->add_destructor_callback(update_in_db);
		}
  }

private:
	sqlite3 *db = nullptr;
  
};

}

extern "C" fpsi::Plugin *construct_plugin(const std::string &plugin_name, const json &plugin_config) {
  return new fpsi::SQLiteDB(plugin_name, plugin_config);
}
