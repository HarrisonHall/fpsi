LOCAL_LIBS = -I./fpsi/src -I./fpsi/include
INCLUDE_LOC = -I./fpsi/include/yaml-cpp/include/yaml-cpp -I./fpsi/include/yaml-cpp/build -L./fpsi/include/yaml-cpp/build
LIBS = $(LOCAL_LIBS) $(INCLUDE_LOC)


default: all

all: counter.cpp
	$(CXX) $(LIBS) -g -std=c++17 -c -fPIC counter.cpp -o counter.o
	$(CXX) $(LIBS) -g -std=c++17 counter.o -shared -o counter.so

clean:
	rm -f *.o *.so
