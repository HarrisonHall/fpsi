LOCAL_LIBS = -I./include/fpsi/src -I./include/fpsi/include
INCLUDE_LOC = -I./include/fpsi/include/yaml-cpp/include/yaml-cpp -I./include/fpsi/include/yaml-cpp/build -L./include/fpsi/include/yaml-cpp/build
ZMQ_LIBS = -I./include/libzmq/include -lsodium -lgnutls
LIBS = $(LOCAL_LIBS) $(INCLUDE_LOC) $(ZMQ_LIBS) -I./include
CXXFLAGS = -std=c++17 -g


default: all

all: zeromq-socket.cpp
	$(CXX) $(LIBS) $(CXXFLAGS) -c -fPIC zeromq-socket.cpp -o zeromq-socket.o
	$(CXX) $(LIBS) $(CXXFLAGS) zeromq-socket.o include/libzmq/build/lib/libzmq.a -shared -o zeromq-socket.so

clean:
	rm -f *.o *.so
