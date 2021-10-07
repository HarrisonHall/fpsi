
default: all

filesim.so: file_sim.cpp
	g++ -c -fPIC file_sim.cpp -o file_sim.o
	g++ file_sim.o -shared -o filesim.so

all: filesim.so


clean:
	rm -f *.o
