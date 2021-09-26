LIBS = -L./include -I./include/yaml-cpp/build
CXXFLAGS = -o fpsi -lyaml-cpp -ldl -O3 -Wall
CXXFILES =  src/main.cpp
GUIFLAGS = -DGUI

all:
	$(CXX) $(LIBS) $(CXXFILES) $(CXXFLAGS)

gui:
	$(CXX) $(GUIFLAGS) $(LIBS) $(CXXFILES) $(CXXFLAGS) `pkg-config gtkmm-3.0 --cflags --libs`

clean:
	rm -f fpsi
