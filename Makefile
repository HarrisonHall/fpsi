LIBS = -L./include -I./include/yaml-cpp/build
CXXFLAGS = -lyaml-cpp -ldl -Wall -std=c++17 -pthread -Wl,--export-dynamic
#CXXFILES =  src/main.cpp
GUIFLAGS = -DGUI
# -O3

.PHONY: clean all minimal main

all: main

dataframe.o: src/data/dataframe.cpp src/data/dataframe.hpp
	$(CXX) -Wall -c src/data/dataframe.cpp -o dataframe.o

datahandler.o: src/data/datahandler.cpp src/data/datahandler.hpp
	$(CXX) -Wall -c src/data/datahandler.cpp -o datahandler.o

glade.o: src/gui/fpsi_gui.glade
	$(LD) -r -b binary -o glade.o src/gui/fpsi_gui.glade

logo.o: src/gui/logo.png src/gui/logo.xml
	glib-compile-resources --generate-source src/gui/logo.xml
	$(CC) -c src/gui/logo.c -o logo.o `pkg-config --libs --cflags glib-2.0`

gui.o: src/gui/gui.cpp src/gui/gui.hpp
	$(CXX) -Wall -c src/gui/gui.cpp -o gui.o `pkg-config gtkmm-3.0 --cflags --libs`

gfx.o: gui.o logo.o glade.o
	$(LD) -r gui.o logo.o glade.o -o gfx.o

plugin.o: src/plugin/plugin.hpp src/plugin/plugin_handler.cpp src/plugin/plugin_handler.hpp
	$(CXX) -Wall -c src/plugin/plugin_handler.cpp -o plugin.o

sqlite.o:
	$(CC) -c include/sqlite3.c -o sqlite.o

util.o: src/util/logging.cpp src/util/logging.hpp src/util/time.cpp src/util/time.hpp
	$(CXX) -Wall -c src/util/logging.cpp -o logging.o
	$(CXX) -Wall -c src/util/time.cpp -o time.o
	ld -r logging.o time.o -o util.o

session.o: src/session/session.cpp src/session/session.hpp
	$(CXX) -Wall -c src/session/session.cpp -o session.o

sessiongui.o: src/session/session.cpp src/session/session.hpp
	$(CXX) -Wall $(GUIFLAGS) -c src/session/session.cpp -o sessiongui.o `pkg-config gtkmm-3.0 --cflags --libs`

minimal: dataframe.o datahandler.o plugin.o session.o sqlite.o util.o
	$(CXX) $(LIBS) dataframe.o datahandler.o session.o sqlite.o util.o src/main.cpp $(CXXFLAGS) -o fpsi-nox

main: dataframe.o datahandler.o gfx.o plugin.o sessiongui.o sqlite.o util.o
	$(CXX) $(LIBS) $(GUIFLAGS) dataframe.o datahandler.o gfx.o plugin.o sessiongui.o sqlite.o util.o src/main.cpp $(CXXFLAGS) `pkg-config gtkmm-3.0 --cflags --libs` -o fpsi

clean:
	rm -f *.o
