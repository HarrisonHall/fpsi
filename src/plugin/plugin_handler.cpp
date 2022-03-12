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
  
  void *so_handle = dlopen(real_plugin_location.c_str(), RTLD_LAZY | RTLD_GLOBAL);
  if (!so_handle) {
    util::log(util::warning, "Unable to open handle to so");
    return nullptr;
  }
  
  void *acquire_func = dlsym(so_handle, "construct_plugin");
  if (!acquire_func) {
    util::log(util::warning, "Unable to get function for construct_plugin");
    return nullptr;
  }

  Plugin *(*acq)(const std::string &, const std::string &, const json &);
  acq = (Plugin *(*)(const std::string &, const std::string &, const json &))acquire_func;
  Plugin *new_plugin = acq(plugin_name, real_plugin_location, plugin_info.value<json>("config", json::object()));
  if (!new_plugin) {
    util::log(util::warning, "Unable to initialize plugin object");
    return nullptr;
  }

  so_handles.push_back(so_handle);

  return std::shared_ptr<Plugin>(new_plugin);
}
  
}
