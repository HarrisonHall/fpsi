LIBS = -L./include -I./include/yaml-cpp/build
CXXFLAGS = -o fpsi -lyaml-cpp -ldl -O3 -Wall -std=c++17
CXXFILES =  src/main.cpp
GUIFLAGS = -DGUI
OBJFILES = src/data/dataframe.o src/data/datahandler.o

all: main

session: src/session/session.cpp
	g++ -Wall -c src/session/session.cpp -o src/session/session.o

sessiongui: src/session/session.cpp src/gui/gui.hpp
	g++ -Wall $(GUIFLAGS) -c src/session/session.cpp -o src/session/sessiongui.o `pkg-config gtkmm-3.0 --cflags --libs`

main: $(OBJFILES) session
	$(CXX) $(LIBS) $(CXXFILES) $(OBJFILES) src/session/session.o $(CXXFLAGS)

gui: $(OBJFILES) sessiongui
	$(CXX) $(LIBS) $(CXXFILES) $(OBJFILES) src/session/sessiongui.o $(CXXFLAGS) `pkg-config gtkmm-3.0 --cflags --libs`

clean:
	rm -f fpsi $(OBJFILES)
