LOCAL_LIBS = -I./fpsi/src -I./fpsi/include
INCLUDE_LOC = -I./fpsi/include/yaml-cpp/include/yaml-cpp -I./fpsi/include/yaml-cpp/build -L./fpsi/include/yaml-cpp/build
LIBS = $(LOCAL_LIBS) $(INCLUDE_LOC) -I./include


default: all

sqlite.o: include/sqlite3.c include/sqlite3.h
	$(CC) -c include/sqlite3.c -o sqlite.o -fPIC

all: sqlite-db.cpp sqlite.o
	$(CXX) $(LIBS) -c -fPIC sqlite-db.cpp -o sqlite-db.o
	$(CXX) $(LIBS) sqlite-db.o sqlite.o -shared -o sqlite-db.so

clean:
	rm -f *.o *.so
