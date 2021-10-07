
#include <iostream>
#include <memory>

#include <gdkmm/pixbuf.h>

#include "../session/session.hpp"
#include "../util/logging.hpp"

#include "gui.hpp"


namespace fpsi {

FPSIWindow::FPSIWindow(BaseObjectType *obj, Glib::RefPtr<Gtk::Builder> const & builder)
  : Gtk::ApplicationWindow(obj), builder{builder}
{
  this->builder->get_widget("about-window", this->about_window);
  Glib::RefPtr<Gdk::Pixbuf> ref_p(Gdk::Pixbuf::create_from_resource("/fpsi/src/gui/logo.png"));
  this->about_window->set_logo(ref_p);
  
  
  Gtk::MenuItem *about_menu_button = 0;
  this->builder->get_widget("main-menu-about", about_menu_button);
  about_menu_button->signal_activate().connect([this](){
    this->about_window->run();
    this->about_window->hide();
  });

  Gtk::MenuItem *quit_menu_button = 0;
  this->builder->get_widget("quit-button", quit_menu_button);
  quit_menu_button->signal_activate().connect([this](){
    this->close();
  });

  //this->about_window->signal_cancel().connect([this](){this->about_window->hide();});
  //this->about_window->signal_delete().connect([this](){this->about_window->hide();});
  //this->about_window->signal_remove().connect([this](){this->about_window->hide();});
  //this->about_window->signal_focus();
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
  
  session.gui_thread = nullptr;
  session.finish();
  
  return;
}

}
