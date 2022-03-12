LOCAL_LIBS = -I./src -I./include
INCLUDE_LOC = -I./include/yaml-cpp/include -I./include/yaml-cpp/include/yaml-cpp -I./include/yaml-cpp/build/include -L./include/yaml-cpp/build
INCLUDE_LIBS = -ldl
LIBS = $(LOCAL_LIBS) $(INCLUDE_LOC) $(INCLUDE_LIBS)
CXXFLAGS = -Wall -std=c++17 -pthread -Wl,--export-dynamic -g
OPT = -Ofast  # -O3
OBJECTS = dataframe.o datahandler.o plugin.o session.o util.o
RELEASE_FLAGS = -fPIE -fPIC # -static -fstack-protector-all
DEBUG_FLAGS = -no-pie

.PHONY: clean all minimal main fpsi

all: debug

fpsi: release

dataframe.o: src/data/dataframe.cpp src/data/dataframe.hpp
	$(CXX) $(CXXFLAGS) $(LIBS) -Wall -c src/data/dataframe.cpp -o dataframe.o

datahandler.o: src/data/datahandler.cpp src/data/datahandler.hpp
	$(CXX) $(CXXFLAGS) $(LIBS) -Wall -c src/data/datahandler.cpp -o datahandler.o

plugin.o: src/plugin/plugin.hpp src/plugin/plugin_handler.cpp src/plugin/plugin_handler.hpp
	$(CXX) $(CXXFLAGS) $(LIBS) -Wall -c src/plugin/plugin_handler.cpp -o plugin.o

util.o: src/util/logging.cpp src/util/logging.hpp src/util/time.cpp src/util/time.hpp
	$(CXX) $(CXXFLAGS) $(LIBS) -Wall -c src/util/logging.cpp -o logging.o
	$(CXX) $(CXXFLAGS) $(LIBS) -Wall -c src/util/time.cpp -o time.o
	$(LD) -r logging.o time.o -o util.o

session.o: src/session/session.cpp src/session/session.hpp
	$(CXX) $(CXXFLAGS) $(LIBS) -Wall -c src/session/session.cpp -o session.o $(CXXFLAGS)

debug: src/main.cpp $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OPT) $(LIBS) $(OBJECTS) $(DEBUG_FLAGS) src/main.cpp include/yaml-cpp/build/libyaml-cpp.a -o fpsi

release: src/main.cpp $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OPT) $(LIBS) $(OBJECTS) $(RELEASE_FLAGS) src/main.cpp include/yaml-cpp/build/libyaml-cpp.a -o fpsi
	#strip fpsi

clean:
	rm -f *.o
