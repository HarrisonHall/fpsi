
default: all

all: counter.cpp
	g++ -c -fPIC counter.cpp -o counter.o
	g++ counter.o -shared -o counter.so

clean:
	rm -f *.o
