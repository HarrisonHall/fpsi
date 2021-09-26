# FPSI
FPSI is a system designed primarily for handling the data and state system of a
sounding rocket. The core program contains a system for data collection, data
filtering, managing states, and communication. A plugin framework is also
provided so that users can dynamically modify and add features to the FPSI.

## Install
```bash
git submodule init
git submodule update
# TODO: Go into include/yaml-cpp and build
make fpsi
./fpsi
```
