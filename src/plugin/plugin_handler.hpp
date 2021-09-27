/*
  plugin_handler.hpp
 */

#pragma once

#include <dlfcn.h>

#include <filesystem>
#include <map>
#include <string>
#include <vector>
#include <utility>

#include "../../include/yaml.h"
 #include "../../plugins/plugin.hpp"

#include "../fpsi.hpp"


namespace fpsi {

size_t MAX_PLUGINS = 1024;

std::vector<void *> so_handles;
//std::map<void *, Plugin *> so_plugin_to_handle;
std::vector<Plugin *> plugin_objects;

Plugin *create_plugin(const std::string &plugin_name, const json &plugin_config) {
  std::string plugin_file = plugin_config.value<std::string>("path", "");
  std::string real_plugin_location = "./" + plugin_file;
  std::filesystem::path f(real_plugin_location);
  if (!std::filesystem::exists(f)) {
    std::cerr << "File " << f << " does not exist" << std::endl;
    return nullptr;
  }
  
  void *so_handle = dlopen(real_plugin_location.c_str(), RTLD_LAZY);
  if (!so_handle) {
    std::cerr << "Unable to open handle to so" << std::endl;
    return nullptr;
  }
  
  void *acquire_func = dlsym(so_handle, "construct_plugin");
  if (!acquire_func) {
    std::cerr << "Unable to get function for construct_plugin" << std::endl;
    return nullptr;
  }

  Plugin *(*acq)(const json &);
  acq = (Plugin *(*)(const json &))acquire_func;
  Plugin *new_plugin = acq(plugin_config);
  if (!new_plugin) {
    std::cerr << "Unable to initialize plugin object" << std::endl;
    return nullptr;
  }

  so_handles.push_back(so_handle);
  //so_handle_to_plugins[so_handle] = new_plugin;
  plugin_objects.push_back(new_plugin);

  return new_plugin;
}

void load_plugins(const std::vector<std::pair<std::string, json>> &plugins){
  for (auto p : plugins) {
    if (!create_plugin(p.first, p.second)) {
      std::cerr << "Unable to create plugin for " << p.first << std::endl;
    }
  }
}
  
}
