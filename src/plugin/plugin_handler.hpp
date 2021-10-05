/*
  plugin_handler.hpp
 */

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "plugin.hpp"

#include "../fpsi.hpp"


namespace fpsi {

extern size_t MAX_PLUGINS;
class Session;

extern std::vector<void *> so_handles;

std::shared_ptr<Plugin> create_plugin(Session *session, const std::string &plugin_name, const json &plugin_config);

std::vector<std::shared_ptr<Plugin>> load_plugins(Session *session, const std::vector<std::pair<std::string, json>> &plugins);

void aggregate_data_threads(Session *session, const std::vector<std::shared_ptr<Plugin>> &plugins);

void change_state(Session *session, const std::vector<std::shared_ptr<Plugin>> &plugins,
                  const std::string &key, const json &last, const json &next);
  
}
