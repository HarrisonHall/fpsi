# sqlite-db
This plugins allows aggregated dataframes and states to be saved to a sqlite
database. This plugin also restores previous dataframes of each source at 
startup.

## Building
1. Build sqlite `cd include/sqlite && mkdir build && cd build && ../configure && make -j 4 && make sqlite3.c`
2. Build sqlite-db.so `make`

## Config
Set `db-path` to modify the sqlite database location. Use `:memory:` to keep the
database in memory.

## Usage
No extra usage is provided.

## TODO
- Allow configurable time limits on restoring data at startup
