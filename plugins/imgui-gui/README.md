# imgui-gui
This plugin provides a graphical user interfaces for using fpsi. By default,
a table of raw and aggregated dataframes is provided along with state information
and the ability to modify plugins.

## Configuration
None.

## Usage
To render extra windows during the gui update cycle, call
`register_gui(...)` after obtaining the imgui-gui plugin
via `::fpsi::session->get_plugin(...)`. 
