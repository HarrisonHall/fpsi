// sqlite-db.cpp

#include "plugin/plugin.hpp"

#include <cassert>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <string>
#include <thread>

#include "sqlite3.h"

#include "session/session.hpp"
#include "data/datahandler.hpp"
#include "util/logging.hpp"
#include "util/time.hpp"


namespace fpsi {

class SQLiteDB : public Plugin {
public:
  SQLiteDB(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config) :
		Plugin(plugin_name, plugin_path, plugin_config) {

		// Read config
		std::string db_path = plugin_config.value<std::string>("db_path", "/tmp/fpsi.sqlite");
		size_t max_recent_data = plugin_config.value<size_t>("max_recent_data", 10);

		// Set up database
		sqlite3_stmt *statement;
		int failed = 0;
		int row_cursor = 0;
		failed = sqlite3_open(db_path.c_str(), &this->db);
		if (failed) {
			this->db = nullptr;
			return;
		}

		this->set_up_dataframes_table();
		this->set_up_states_table();

		// Read initial data
		for (const auto &packet_type : {"agg", "stt"}) {
			std::vector<std::string> packet_sources;
			std::string get_sources = "SELECT DISTINCT df_source FROM dataframes";
			failed = sqlite3_prepare_v2(this->db, get_sources.c_str(), get_sources.size(), &statement, NULL);
			row_cursor = sqlite3_step(statement);
			while (row_cursor == SQLITE_ROW) {
				packet_sources.push_back(
					{reinterpret_cast<const char*>(sqlite3_column_text(statement, 0))});
				row_cursor = sqlite3_step(statement);  // Advance
			}
			sqlite3_finalize(statement);

			for (const auto &packet_source : packet_sources) {
				std::string get_data =
					"SELECT * FROM "
					"(SELECT * FROM dataframes where df_source = ? AND df_type = ? "
					"ORDER BY datetime(df_time) DESC LIMIT " + std::to_string(max_recent_data) + " )"
					"ORDER BY df_id ASC";
				failed = sqlite3_prepare_v2(this->db, get_data.c_str(), get_data.size(), &statement, NULL);
				
				// Bind vars
				failed = sqlite3_bind_text(statement, 1, packet_source.c_str(), -1, SQLITE_TRANSIENT);
				assert(("Failed to bind source", failed == SQLITE_OK));
				failed = sqlite3_bind_text(statement, 2, packet_type, -1, SQLITE_TRANSIENT);
				assert(("Failed to bind type", failed == SQLITE_OK));

				// Iterate
				row_cursor = sqlite3_step(statement);
				while (row_cursor == SQLITE_ROW) {
					// Read columns
					size_t df_id = sqlite3_column_int(statement, 0);
					std::string df_source(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
					std::string df_type(reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)));
					const void *bson_data = sqlite3_column_blob(statement, 3);
					int bson_len = sqlite3_column_bytes(statement, 3);
					std::vector<std::uint8_t> v_bson(bson_len);
					memcpy(v_bson.data(), bson_data, bson_len);
					json df_json = json::from_bson(v_bson);
					std::string df_time(reinterpret_cast<const char*>(sqlite3_column_text(statement, 4)));
					
					::fpsi::session->data_handler->create_data_source(df_source);  // Ensure source
					
					auto df = std::make_shared<DataFrame>(df_id, df_source, df_type, df_json);
					df->set_time(df_time);  // Set correct time
					
					// TODO - only insert older packets if they fit within a recent timeframe
					if (df_type == "raw") {
						::fpsi::session->data_handler->create_raw(df);
					} else if (df_type == "agg") {
						::fpsi::session->data_handler->create_agg(df);
					} else {
						util::log(util::warning, "Invalid dataframe type: %s (id=%d)", df_type.c_str(), df_id);
					}
					
					row_cursor = sqlite3_step(statement);  // Advance
				}
				sqlite3_finalize(statement);
			}
			
		}

		// Get initial states
		{
			// Get unique states
			std::vector<std::string> unique_states;
			std::string get_states = "SELECT DISTINCT state_key FROM states";
			failed = sqlite3_prepare_v2(this->db, get_states.c_str(), get_states.size(), &statement, NULL);
			row_cursor = sqlite3_step(statement);
			while (row_cursor == SQLITE_ROW) {
				unique_states.push_back(reinterpret_cast<const char*>(sqlite3_column_text(statement, 0)));
				row_cursor = sqlite3_step(statement);  // Advance
			}
			sqlite3_finalize(statement);

			// Bring back each state
			for (const std::string &state_key : unique_states) {
				std::string get_state =
					"SELECT * FROM states where state_key = ? "
					"ORDER BY datetime(state_time) DESC LIMIT 1";
				failed = sqlite3_prepare_v2(this->db, get_state.c_str(), get_state.size(), &statement, NULL);
				
				// Bind vars
				failed = sqlite3_bind_text(statement, 1, state_key.c_str(), -1, SQLITE_TRANSIENT);
				assert(("Failed to bind state key", failed == SQLITE_OK));

				row_cursor = sqlite3_step(statement);
				assert(row_cursor == SQLITE_ROW);
				// Read columns
				size_t state_id = sqlite3_column_int(statement, 0);
				std::string _state_key(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
				const void *bson_data = sqlite3_column_blob(statement, 2);
				int bson_len = sqlite3_column_bytes(statement, 2);
				std::vector<std::uint8_t> v_bson(bson_len);
				memcpy(v_bson.data(), bson_data, bson_len);
				json state_data = json::from_bson(v_bson);
				std::string state_time(reinterpret_cast<const char*>(sqlite3_column_text(statement, 3)));

				// TODO - only insert older states if they fit within a recent timeframe
				util::log(util::debug, "Restoring state %s to %s", state_key.c_str(), state_data.dump().c_str());
				::fpsi::session->set_state(state_key, state_data);
				
				sqlite3_finalize(statement);
			}
		}
  }

  ~SQLiteDB() {
		if (this->db) {
			util::log("Closing sqlitedb");
			sqlite3_close(db);  // Close database
		}
  }

  void post_aggregate(const std::map<std::string, std::vector<std::shared_ptr<DataFrame>>> &agg_data) {
		for (auto [source_name, dfs] : agg_data)
			for (auto df : dfs)
				this->update_df(df);
  }

	void post_state(const std::map<std::string, std::shared_ptr<DataFrame>> &agg_data) {
		for (auto [source_name, df] : agg_data)
			this->update_df(df);
  }

	// Called when a state changes (from ::fpsi::session->set_state(...))
  virtual void state_change(const std::string &key, const json &last, const json &next) {
		std::string insert_state =
			"INSERT INTO states(state_key, state_data, state_time) "
			"VALUES(?, ?, ?)";
		sqlite3_stmt *statement;
		int failed = sqlite3_prepare_v2(this->db, insert_state.c_str(), insert_state.size(), &statement, NULL);
		
		// Bind vars
		failed = sqlite3_bind_text(statement, 1, key.c_str(), -1, SQLITE_TRANSIENT);
		assert(("Failed to bind state_key", failed == SQLITE_OK));

		auto state_data = json::to_bson(next);
		failed = sqlite3_bind_blob(statement, 2, state_data.data(), state_data.size(), SQLITE_TRANSIENT);
		assert(("Failed to bind blob", failed == SQLITE_OK));
		auto state_time = util::to_time_str(util::timestamp());
		failed = sqlite3_bind_text(statement, 3, state_time.c_str(), -1, SQLITE_TRANSIENT);
		assert(("Failed to bind time", failed == SQLITE_OK));
		
		// Exec
		failed = sqlite3_step(statement);
		assert(("sqlite insert is a single operation", failed = SQLITE_DONE));
		sqlite3_finalize(statement);
	}

private:
	sqlite3 *db = nullptr;

	void update_df(std::shared_ptr<DataFrame> df) {
		const std::string sqlite_name = this->name;
		static auto update_in_db = [this, sqlite_name](DataFrame *df) {
			if (!::fpsi::session->get_plugin(sqlite_name)) return;  // sqlite has been unloaded
			// Prepare update
			std::string update_df =
				"UPDATE dataframes SET df_source = ?, df_type = ?, df_data = ?, df_time = ? "
				"WHERE df_id = ?";
			sqlite3_stmt *statement;
			int failed = sqlite3_prepare_v2(this->db, update_df.c_str(), update_df.size(), &statement, NULL);

			// Bind vars
			failed = sqlite3_bind_text(statement, 1, df->get_source().c_str(), -1, SQLITE_TRANSIENT);
			assert(("Failed to bind source", failed == SQLITE_OK));
			failed = sqlite3_bind_text(statement, 2, df->get_type().c_str(), -1, SQLITE_TRANSIENT);
			assert(("Failed to bind text", failed == SQLITE_OK));
			auto df_bson = df->get_bson();
			failed = sqlite3_bind_blob(statement, 3, df_bson.data(), df_bson.size(), SQLITE_TRANSIENT);
			assert(("Failed to bind blob", failed == SQLITE_OK));
			failed = sqlite3_bind_text(statement, 4, df->get_time().c_str(), -1, SQLITE_TRANSIENT);
			assert(("Failed to bind time", failed == SQLITE_OK));
			failed = sqlite3_bind_int(statement, 5, df->get_id());
			assert(("Failed to bind int", failed == SQLITE_OK));

			// Exec
			failed = sqlite3_step(statement);
			assert(("sqlite insert is a single operation", failed = SQLITE_DONE));
			sqlite3_finalize(statement);

			util::log(util::debug, "Updated df %d in sqlite", df->get_id());
			return;
		};

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
		auto df_bson = df->get_bson();
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

	void set_up_dataframes_table() {
		sqlite3_stmt *statement;
		int failed = 0;
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
	}

	void set_up_states_table() {
		sqlite3_stmt *statement;
		int failed = 0;
		// Ensure dataframes table exists
		std::string check_table = "SELECT name FROM sqlite_master WHERE name ='states' and type='table'";
		bool table_exists = false;
		int row_cursor = 0;
		failed = sqlite3_prepare_v2(this->db, check_table.c_str(), check_table.size(), &statement, NULL);
		row_cursor = sqlite3_step(statement);
		while (row_cursor == SQLITE_ROW) {
			const char *name = reinterpret_cast<const char *>(sqlite3_column_text(statement, 0));
			table_exists = !strncmp(name, "states", 6);
			row_cursor = sqlite3_step(statement);
		}
		sqlite3_finalize(statement);

		if (!table_exists) {
			std::string create_table = 
				"CREATE TABLE states "
				"(state_id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"state_key TEXT, state_data BLOB, state_time TEXT)";
			failed = sqlite3_exec(this->db, create_table.c_str(), NULL, NULL, NULL);
			util::log(util::debug, "Created table states");
		} else {
			util::log(util::debug, "states table exists");
		}
	}
  
};

}

extern "C" fpsi::Plugin *construct_plugin(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config) {
  return new fpsi::SQLiteDB(plugin_name, plugin_path, plugin_config);
}
