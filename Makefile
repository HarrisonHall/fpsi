LIBS = -L./include -I./include/yaml-cpp/build
CXXFLAGS = -o fpsi -lyaml-cpp -ldl -O3 -Wall -std=c++17
#CXXFILES =  src/main.cpp
GUIFLAGS = -DGUI

.PHONY: clean all minimal main

all: main

dataframe.o: src/data/dataframe.cpp
	g++ -Wall -c src/data/dataframe.cpp -o dataframe.o

datahandler.o: src/data/datahandler.cpp
	g++ -Wall -c src/data/datahandler.cpp -o datahandler.o

session.o: src/session/session.cpp
	g++ -Wall -c src/session/session.cpp -o session.o

glade.o: src/gui/fpsi_gui.glade
	ld -r -b binary -o glade.o src/gui/fpsi_gui.glade

gui.o: src/gui/gui.cpp
	g++ -Wall -c src/gui/gui.cpp -o gui.o `pkg-config gtkmm-3.0 --cflags --libs`

sessiongui.o: src/session/session.cpp
	g++ -Wall $(GUIFLAGS) -c src/session/session.cpp -o sessiongui.o `pkg-config gtkmm-3.0 --cflags --libs`

minimal: dataframe.o datahandler.o session.o
	$(CXX) $(LIBS) $(CXXFILES) dataframe.o datahandler.o session.o src/main.cpp $(CXXFLAGS)

main: dataframe.o datahandler.o glade.o sessiongui.o gui.o
	$(CXX) $(LIBS) $(GUIFLAGS) glade.o sessiongui.o gui.o src/main.cpp $(CXXFLAGS) `pkg-config gtkmm-3.0 --cflags --libs`

clean:
	rm -f *.o
