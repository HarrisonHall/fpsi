# FPSI
FPSI is a system designed primarily for handling local data systems managed by
multiple nodes. The core program contains a system for data collection, data
filtering, managing states, and communication. A plugin framework is also
provided so that users can dynamically modify and add features to FPSI.

FPSI was designed abstract so I could use it for a couple of personal projects.
It is hard to know just what FPSI can be used for without an example. When one
of my projects are sufficiently developed I will link it here.

## Install
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
