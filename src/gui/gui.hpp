
#pragma once

#include <gtkmm.h>
#include <gtkmm/application.h>
#include <gtkmm/aboutdialog.h>
#include <gtkmm/dialog.h>
#include <glibmm/propertyproxy.h>

#include "../session/session.hpp"
#include "../fpsi.hpp"

extern char _binary_src_gui_fpsi_gui_glade_start[];
extern char _binary_src_gui_fpsi_gui_glade_end[];

namespace fpsi {

class FPSIWindow : public Gtk::ApplicationWindow {
  
public:
  FPSIWindow(BaseObjectType *obj, Glib::RefPtr<Gtk::Builder> const & builder);
  ~FPSIWindow();
  void press_debug_button(Gtk::Button *butt);

//private:
  Glib::RefPtr<Gtk::Builder> builder;
  Gtk::AboutDialog *about_window = 0;
  
};

void gui_build(Session &session);

}
