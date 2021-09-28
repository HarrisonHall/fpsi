
#pragma once

#include <gtkmm.h>

#include "../session/session.hpp"
#include "../fpsi.hpp"

extern char _binary_src_gui_fpsi_gui_glade_start[];
extern char _binary_src_gui_fpsi_gui_glade_end[];

namespace fpsi {

class FPSIWindow : public Gtk::ApplicationWindow {
  //Glib::RefPtr<Gtk::Builder> builder;
  //Gtk::Window *window1;
  
public:
  FPSIWindow(BaseObjectType *obj, Glib::RefPtr<Gtk::Builder> const & builder);
  ~FPSIWindow();
  void press_debug_button(Gtk::Button *butt);

private:
  Glib::RefPtr<Gtk::Builder> builder;
  
};

void gui_build(Session &session);

}
