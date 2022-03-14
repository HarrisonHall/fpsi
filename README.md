# FPSI
FPSI is a system designed primarily for handling local data systems managed by
multiple nodes. The core program contains a system for data collection, data
filtering, managing states, and communication. A plugin framework is also
provided (and strictly necessary for nontrivial usage) so that users can
dynamically modify and add features to FPSI.

FPSI was designed abstract so I could use it for a couple of personal projects.
It is hard to know just what FPSI can be used for without an example. When one
of my projects are sufficiently developed I will link it here.

## Running
### Installation
```bash
# Pull submodules
git submodule update --init --recursive --depth 1

# Build yaml-cpp
cd include/yaml-cpp
mkdir build
cd build
cmake ..
make
cd ../..

# Build fpsi
make fpsi

# Run
./fpsi
```

### Examples
`plugins/examples/` provides a few examples using the general-use plugins.

## Concepts
- FPSI has a global session object
- Data is provided to the session by creating a data source and supplying raw dataframes
- Data is aggregated at a time step and the latest aggregated frame is supplied back
  during the post-aggregation step
- ...

## FAQ
> Does this compile on windows/macos?
This has only been fully tested on arch linux. Likely not as-is, but there is nothing
inherently linux-only in fpsi.
> Future work?
Aside from ironing out TODOs, the future work is really divided into a) making 
general-use plugins as they are needed and b) figuring out a way to generalize the
data/state system. In the future a version 2.0 might have allow the data/state 
system to be abstracted into plugins, but the current implementation works for now.
Otherwise, there is certainly room for optimization.
