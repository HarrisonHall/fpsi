LIBS = -L./include -I./include/yaml-cpp/build
CXXFLAGS = -o fpsi -lyaml-cpp -ldl -O3
CXXFILES =  src/main.cpp

all:
	$(CXX) $(LIBS) $(CXXFILES) $(CXXFLAGS)

clean:
	rm -f fpsi
