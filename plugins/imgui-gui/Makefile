LOCAL_LIBS = -I./fpsi/src -I./fpsi/include -I./imgui -I./imgui/backends
INCLUDE_LOC = -I./fpsi/include/yaml-cpp/include/yaml-cpp -I./fpsi/include/yaml-cpp/build -L./fpsi/include/yaml-cpp/build
GFX_LIBS = -lglfw -lrt -lm -ldl -lX11 -lpthread -lxcb -lXau -lXdmcp -lGL
LIBS = $(LOCAL_LIBS) $(GFX_LIBS) $(INCLUDE_LOC) -I./include
CXXFLAGS = --std=c++17 -g -Wall -Wformat -fPIC

# `pkg-config --static --libs glfw3` `pkg-config --cflags glfw`

default: all

imgui-rend.o:
	$(CXX) $(LIBS) $(CXXFLAGS) -c imgui/backends/imgui_impl_glfw.cpp -o imgui-rend.o

imgui-gfx.o:
	$(CXX) $(LIBS) $(CXXFLAGS) -c imgui/backends/imgui_impl_opengl3.cpp -o imgui-gfx.o

imgui-general.o:
	$(CXX) $(LIBS) $(CXXFLAGS) -c imgui/imgui_draw.cpp -o imgui-draw.o
	$(CXX) $(LIBS) $(CXXFLAGS) -c imgui/imgui_tables.cpp -o imgui-tables.o
	$(CXX) $(LIBS) $(CXXFLAGS) -c imgui/imgui_widgets.cpp -o imgui-widgets.o
	$(CXX) $(LIBS) $(CXXFLAGS) -c imgui/imgui.cpp -o imgui.o
	ld -r imgui-draw.o imgui-tables.o imgui-widgets.o imgui.o -o imgui-general.o

all: imgui-gui.cpp imgui-rend.o imgui-gfx.o imgui-general.o
	$(CXX) $(LIBS) $(CXXFLAGS) -shared imgui-gui.cpp imgui-general.o imgui-gfx.o imgui-rend.o -o imgui-gui.so

clean:
	rm -f *.o *.so
