
#include <iostream>
#include <memory>

#include "../session/session.hpp"

#include "gui.hpp"

namespace fpsi {

FPSIWindow::FPSIWindow(BaseObjectType *obj, Glib::RefPtr<Gtk::Builder> const & builder):
  Gtk::ApplicationWindow(obj), builder{builder}
{

}
FPSIWindow::~FPSIWindow() {}

void FPSIWindow::press_debug_button(Gtk::Button *butt) {
  std::cout << "Button pressed!" << std::endl;
  return;
}

void gui_build(Session &session) {
  auto app = Gtk::Application::create("fpsi.mainwindow");

  if (session.get_glade_file().empty()) {
    std::cout << "using embedded glade file" << std::endl;
    //Glib::RefPtr<Gtk::Builder> builder(Gtk::Builder::create_from_file("./src/gui/fpsi_gui.glade"));
    Glib::RefPtr<Gtk::Builder> builder(Gtk::Builder::create_from_string(_binary_src_gui_fpsi_gui_glade_start));
    FPSIWindow *win;
    builder->get_widget_derived("main-window", win);
    app->run(*win);
  } else {
    std::cout << "using other glade file" << std::endl;
    Glib::RefPtr<Gtk::Builder> builder(Gtk::Builder::create_from_file(session.get_glade_file()));
    //Glib::RefPtr<Gtk::Builder> builder(Gtk::Builder::create_from_string(session.get_glade_file()));
    FPSIWindow *win;
    builder->get_widget_derived("main-window", win);
    app->run(*win);
  }

  // TODO - kill fpsi
  session.gui_thread = nullptr;
  session.finish();
  
  return;
}

}
