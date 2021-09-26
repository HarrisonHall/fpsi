#include "plugin.hpp"


namespace fpsi {
  
class DebugPlug : public Plugin {
public:
  DebugPlug() {
    
  }
  
  virtual ~DebugPlug() {
    
  }

  virtual int foo() {
    return 1;
  }

  virtual double bar() {
    return 3.5;
  }

  virtual std::string foobar() {
    return "hello world!";
  }
  
};

}

extern "C" fpsi::Plugin *acquire() {
  return new fpsi::DebugPlug();
}
