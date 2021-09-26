#include <iostream>



namespace fpsi {
  
class Plugin {
public:
  Plugin() {
    
  }
  
  virtual ~Plugin() {
    
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

extern "C" fpsi::Plugin *acquire();

extern "C" void destroy(fpsi::Plugin *plug) {
  delete plug;
}
