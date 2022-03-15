LOCAL_LIBS = -I./include/fpsi/src -I./include/fpsi/include
INCLUDE_LOC = -I./include/fpsi/include/yaml-cpp/include/ -I./include/fpsi/include/yaml-cpp/build/include -I./include/fpsi/include/yaml-cpp/include/yaml-cpp/include -I./include/fpsi/include/yaml-cpp/build/include/yaml-cpp
LIBS = $(LOCAL_LIBS) $(INCLUDE_LOC)

default: all

all: template.cpp
	$(CXX) $(LIBS) -g -std=c++17 -c -fPIC template.cpp -o template.o
	$(CXX) $(LIBS) -g -std=c++17 template.o -shared -o template.so

clean:
	rm -f *.o *.so
