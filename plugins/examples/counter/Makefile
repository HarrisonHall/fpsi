LOCAL_LIBS = -I./include/fpsi/src -I./include/fpsi/include
INCLUDE_LOC = -I./include/fpsi/include/yaml-cpp/include/yaml-cpp -I./include/fpsi/include/yaml-cpp/include -I./include/fpsi/include/yaml-cpp/build/include/yaml-cpp -I./include/fpsi/include/yaml-cpp/build/include
LIBS = $(LOCAL_LIBS) $(INCLUDE_LOC)
IMGUI_LIBS = -I./include/fpsi/plugins/imgui-gui/include/imgui -I./include/fpsi/plugins/imgui-gui/include/implot -I./include/fpsi/plugins/imgui-gui


default: all

all: counter.cpp
	$(CXX) $(LIBS) -g -std=c++17 -c -fPIC counter.cpp -o counter.o
	$(CXX) $(LIBS) -g -std=c++17 counter.o -shared -o counter.so

gui: counter.cpp
	$(CXX) $(LIBS) $(IMGUI_LIBS) -DGUI -g -std=c++17 -c -fPIC counter.cpp -o counter.o
	$(CXX) $(LIBS) -DGUI -g -std=c++17 counter.o -shared -o counter.so

clean:
	rm -f *.o *.so
