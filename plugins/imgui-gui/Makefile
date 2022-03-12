LOCAL_LIBS = -I./include/fpsi/src -I./include/fpsi/include
INCLUDE_LOC = -I./include/fpsi/include/yaml-cpp/include/yaml-cpp -I./include/fpsi/include/yaml-cpp/build -L./include/fpsi/include/yaml-cpp/build
IMGUI_INC = -I./include/imgui -I./include/imgui/backends
IMPLOT_INC = -I./include/implot
GFX_LIBS = -lglfw -lrt -lm -ldl -lX11 -lpthread -lxcb -lXau -lXdmcp -lGL
LIBS = $(LOCAL_LIBS) $(GFX_LIBS) $(INCLUDE_LOC) $(IMGUI_INC) $(IMPLOT_INC) -I./include
CXXFLAGS = --std=c++17 -g -Wall -Wformat -fPIC

# `pkg-config --static --libs glfw3` `pkg-config --cflags glfw`

default: all

imgui-rend.o:
	$(CXX) $(LIBS) $(CXXFLAGS) -c include/imgui/backends/imgui_impl_glfw.cpp -o imgui-rend.o

imgui-gfx.o:
	$(CXX) $(LIBS) $(CXXFLAGS) -c include/imgui/backends/imgui_impl_opengl3.cpp -o imgui-gfx.o

imgui-general.o:
	$(CXX) $(LIBS) $(CXXFLAGS) -c include/imgui/imgui_draw.cpp -o imgui-draw.o
	$(CXX) $(LIBS) $(CXXFLAGS) -c include/imgui/imgui_tables.cpp -o imgui-tables.o
	$(CXX) $(LIBS) $(CXXFLAGS) -c include/imgui/imgui_widgets.cpp -o imgui-widgets.o
	$(CXX) $(LIBS) $(CXXFLAGS) -c include/imgui/imgui.cpp -o imgui.o
	ld -r imgui-draw.o imgui-tables.o imgui-widgets.o imgui.o -o imgui-general.o

implot.o:
	$(CXX) $(LIBS) $(CXXFLAGS) -c include/implot/implot.cpp -o implot-raw.o
	$(CXX) $(LIBS) $(CXXFLAGS) -c include/implot/implot_items.cpp -o implot-items.o
	ld -r implot-raw.o implot-items.o -o implot.o

all: imgui-gui.cpp imgui-rend.o imgui-gfx.o imgui-general.o implot.o
	$(CXX) $(LIBS) $(CXXFLAGS) -shared imgui-gui.cpp imgui-general.o imgui-gfx.o imgui-rend.o implot.o -o imgui-gui.so

clean:
	rm -f *.o *.so
