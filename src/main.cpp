/*
  fpsi
 */

#include <iostream>
#include <dlfcn.h>

#include "../include/yaml.h"

#include "../plugins/plugin.hpp"

#include "session.hpp"


fpsi::Plugin *my_acquire(void *so_handle) {
  void *acquire_func = dlsym(so_handle, "acquire");
  std::cout << "got acq " << acquire_func << " " << so_handle << std::endl;
  fpsi::Plugin *(*acq)();
  acq = (fpsi::Plugin *(*)())acquire_func;
  return acq();
}


int main() {
  fpsi::Session s("config.yaml");
  std::cout << s.to_string() << std::endl;

  std::vector<void *> handles;
  std::vector<fpsi::Plugin *> plugs;
  int i = 0;
  for (auto plugin_location : s.get_plugins()) {
    std::string rp = "./" + plugin_location;
    void *so_handle = dlopen(plugin_location.c_str(), RTLD_LAZY);
    std::cout << "wowza " << rp << " " << so_handle  << std::endl;
    handles.push_back(so_handle);
    plugs.push_back(my_acquire(so_handle));
    std::cout << "wow" << plugin_location << " " << plugs[i]->foo() << " " << plugs[i]->bar() << std::endl;
    i++;
  }

  for (auto plug : handles) {
    dlclose(plug);
  }
  return 0;
}
