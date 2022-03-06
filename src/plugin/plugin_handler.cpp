/*
  plugin_handler.hpp
 */

#include <dlfcn.h>

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <utility>

#include "yaml.h"
#include "plugin.hpp"

#include "fpsi.hpp"

#include "session/session.hpp"
#include "data/datasource.hpp"
#include "data/datahandler.hpp"
#include "util/logging.hpp"


namespace fpsi {

size_t MAX_PLUGINS = 1024;

std::vector<void *> so_handles;

std::shared_ptr<Plugin> create_plugin(const std::string &plugin_name, const json &plugin_info) {
  std::string plugin_file = plugin_info.value<std::string>("path", "");
  std::string real_plugin_location = "./" + plugin_file;
  std::filesystem::path f(real_plugin_location);
  if (!std::filesystem::exists(f)) {
    util::log(util::warning, "File %s does not exist", real_plugin_location.c_str());
    return nullptr;
  }
  
  void *so_handle = dlopen(real_plugin_location.c_str(), RTLD_LAZY);
  if (!so_handle) {
    util::log(util::warning, "Unable to open handle to so");
    return nullptr;
  }
  
  void *acquire_func = dlsym(so_handle, "construct_plugin");
  if (!acquire_func) {
    util::log(util::warning, "Unable to get function for construct_plugin");
    return nullptr;
  }

  Plugin *(*acq)(const std::string &, const json &);
  acq = (Plugin *(*)(const std::string &, const json &))acquire_func;
  Plugin *new_plugin = acq(plugin_name, plugin_info.value<json>("config", json::object()));
  if (!new_plugin) {
    util::log(util::warning, "Unable to initialize plugin object");
    return nullptr;
  }

  so_handles.push_back(so_handle);
  //so_handle_to_plugins[so_handle] = new_plugin;
  //plugin_objects.push_back(new_plugin);

  return std::shared_ptr<Plugin>(new_plugin);
}

std::vector<std::shared_ptr<Plugin>> load_plugins(const std::vector<std::pair<std::string, json>> &plugins){
  std::vector<std::shared_ptr<Plugin>> loaded_plugins;
  if (!plugins.size()) {
    util::log(util::warning, "No plugins enabled");
		exit(1);
  }
  for (auto p : plugins) {
    auto new_plugin = create_plugin(p.first, p.second);
    if (!new_plugin) {
      util::log(util::error, "Unable to create plugin for %s", p.first.c_str());
    } else {
      loaded_plugins.push_back(new_plugin);
      util::log(util::debug, "Created plugin for %s", p.first.c_str());
    }
  }
  return loaded_plugins;
}

void aggregate_data_threads(Session *session, const std::vector<std::shared_ptr<Plugin>> &plugins) {
  // Get data
  std::map<std::string, std::vector<std::shared_ptr<DataFrame>>> new_data;
  for (auto ds_name : session->data_handler->get_sources()) {
    new_data[ds_name] = session->data_handler->get_recent_data(ds_name);
  }
  
  // Run pre-aggregate hook for each plugin
  std::vector<std::thread*> threads;
  for (auto plugin : plugins) {
    threads.push_back(new std::thread(&Plugin::pre_aggregate, plugin.get(), new_data));
    //threads.push_back(new std::thread([plugin, new_data](){plugin->pre_aggregate(new_data);}));
  }
  for (auto thread : threads) {
    thread->join();
    delete thread;
  }
  
  // Get data
  std::map<std::string, std::shared_ptr<DataFrame>> agg_data;
  for (auto ds_name : session->data_handler->get_sources()) {
    agg_data[ds_name] = session->data_handler->get_newest_agg(ds_name);
  }

  // Run post-aggregate hook for each plugin
  threads.clear();
  for (auto plugin : plugins) {
    threads.push_back(new std::thread(&Plugin::post_aggregate, plugin.get(), agg_data));
    //threads.push_back(new std::thread([plugin](){plugin->post_aggregate();}));
  }
  for (auto thread : threads) {
    thread->join();
    delete thread;
  }
}

void change_state(Session *session, const std::vector<std::shared_ptr<Plugin>> &plugins,
                  const std::string &key, const json &last, const json &next) {
  std::vector<std::thread *> threads;
  for (auto plugin : plugins) {
    threads.push_back(new std::thread(&Plugin::pre_state_change, plugin.get(), key, last, next));
  }
  for (auto thread : threads) {
    thread->join();
    delete thread;
  }

  threads.clear();
  for (auto plugin : plugins) {
    threads.push_back(new std::thread(&Plugin::post_state_change, plugin.get(), key, last, next));
  }
  for (auto thread : threads) {
    thread->join();
    delete thread;
  }
}
  
}
