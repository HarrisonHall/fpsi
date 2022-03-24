LOCAL_LIBS = -I./include/fpsi/src -I./include/fpsi/include
INCLUDE_LOC = -I./include/fpsi/include/yaml-cpp/include/yaml-cpp -I./include/fpsi/include/yaml-cpp/include/ -I./include/fpsi/include/yaml-cpp/build -L./include/fpsi/include/yaml-cpp/build -I./include/fpsi/include/yaml-cpp/build/include
INCLUDE_SQL = -I./include/sqlite/build
LIBS = $(LOCAL_LIBS) $(INCLUDE_LOC) $(INCLUDE_SQL)
CXXFLAGS = -std=c++17 -g


default: all

sqlite.o: include/sqlite/build/sqlite3.c include/sqlite/build/sqlite3.h
	$(CC) -g -c include/sqlite/build/sqlite3.c -o sqlite.o -fPIC

all: sqlite-db.cpp sqlite.o
	$(CXX) $(LIBS) $(CXXFLAGS) -c -fPIC sqlite-db.cpp -o sqlite-db.o
	$(CXX) $(LIBS) $(CXXFLAGS) sqlite-db.o sqlite.o -shared -o sqlite-db.so

clean:
	rm -f *.o *.so
