#pragma once

#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>
#include <utility>

#include "../fpsi.hpp"


namespace fpsi {

class Session;
class DataFrame;
class DataSource;
class DBHandler;

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
};




}  // namespace fpsi
