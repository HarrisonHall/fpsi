
#pragma once

#include <iostream>
#include <memory>

#include <gtkmm.h>


namespace fpsi {

class FPSIWindow : public Gtk::ApplicationWindow {
  //Glib::RefPtr<Gtk::Builder> builder;
  //Gtk::Window *window1;
  
public:
  FPSIWindow(BaseObjectType *obj, Glib::RefPtr<Gtk::Builder> const & builder):
    Gtk::ApplicationWindow(obj), builder{builder}
  {
    //builder->get_widget("main-window", window1);
    //add(*window1);
    //ui->default_widget(window1);
    //window1->show();
    // show_all();
    //set_default_widget(window1);
  }
  ~FPSIWindow() {
    
  }

  void press_debug_button(Gtk::Button *butt) {
    std::cout << "Button pressed!" << std::endl;
    return;
  }

private:
  Glib::RefPtr<Gtk::Builder> builder;
  
};

void gui_build() {
  auto app = Gtk::Application::create("fpsi.mainwindow");
  //FPSIWindow w;
  //app->run(w);
  Glib::RefPtr<Gtk::Builder> builder(Gtk::Builder::create_from_file("./src/gui/fpsi_gui.glade"));
  FPSIWindow *win;
  builder->get_widget_derived("main-window", win);
  app->run(*win);
  
  
  return;
}

}
