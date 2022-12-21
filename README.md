# FPSI
FPSI is a rust library designed primarily for handling local data systems managed
by multiple nodes. The core program contains a system for data collection, data
filtering, managing states, and communication. A plugin framework is also
provided (and strictly necessary for nontrivial usage) so that users can
dynamically modify and add features to FPSI.

FPSI was designed abstract so I could use it for a couple of personal projects.
It is hard to know just what FPSI can be used for without an example. **When one
of my projects are sufficiently developed I will link it here.**

> Note: The rust rewrite brings with it significant architecture changes.

## Running
### Installation

### Development
`cargo test`

### Examples
- TODO!

## Plugins
The provided plugins (will) provide:
- SQLite data storage
- Imgui GUI
- Dynamic c plugin interface
- ZeroMQ socket communication

## Concepts
- FPSI has a global session object which holds the public config and data_handler
- Plugins that implement `fpsi::plugin::Plugin` are registered with the session
- Data is provided to the session by creating a data source and supplying raw dataframes
- The system runds with `Session::run`
  - An event loop for each plugin is created to process/manage/produce events
  - Raw frames are aggregated at a time step and the aggregated frames are supplied back
    during the post-aggregation step

## FAQ
> Future work?

Optimization, a more robust communication system, and more general-use plugins.

> GUI?

FPSI works completely without the gui. The gui is a plugin that is useful when
fpsi is run locally or is used to manage data from other nodes.
