Note: This repo is being refactored for compatibility with c and rust modules.

# FPSI
FPSI is a system designed primarily for handling local data systems managed by
multiple nodes. The core program contains a system for data collection, data
filtering, managing states, and communication. A plugin framework is also
provided so that users can dynamically modify and add features to FPSI.

## Install
```bash
git submodule init
git submodule update
# TODO: Go into include/yaml-cpp and build
make fpsi
./fpsi
```
