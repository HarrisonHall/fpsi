LOCAL_LIBS = -I./include/fpsi/src -I./include/fpsi/include
INCLUDE_LOC = -I./include/fpsi/include/yaml-cpp/include/yaml-cpp -I./include/fpsi/include/yaml-cpp/build -L./include/fpsi/include/yaml-cpp/build
LIBS = $(LOCAL_LIBS) $(INCLUDE_LOC)
IMGUI_LIBS = -I./include/fpsi/plugins/imgui-gui/include/imgui -I./include/fpsi/plugins/imgui-gui/include/implot -I./include/fpsi/plugins/imgui-gui


default: all

all: echoecho.cpp
	$(CXX) $(LIBS) -g -std=c++17 -c -fPIC echoecho.cpp -o echoecho.o
	$(CXX) $(LIBS) -g -std=c++17 echoecho.o -shared -o echoecho.so

clean:
	rm -f *.o *.so
