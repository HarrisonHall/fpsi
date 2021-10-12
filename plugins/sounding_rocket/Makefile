
default: all

filesim.so: file_sim.cpp
	g++ -c -fPIC file_sim.cpp -o file_sim.o
	g++ file_sim.o -shared -o filesim.so

states.so: state_system.cpp
	g++ -c -fPIC state_system.cpp -o state_system.o
	g++ state_system.o -shared -o states.so

all: filesim.so states.so


clean:
	rm -f *.so
