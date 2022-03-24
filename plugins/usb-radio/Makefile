LOCAL_LIBS = -I./include/fpsi/src -I./include/fpsi/include
INCLUDE_LOC = -I./include/fpsi/include/yaml-cpp/include/ -I./include/fpsi/include/yaml-cpp/build/include -I./include/fpsi/include/yaml-cpp/include/yaml-cpp/include -I./include/fpsi/include/yaml-cpp/build/include/yaml-cpp
INCLUDE_SOAPY = -I./include/SoapySDR/include -L./include/SoapySDR/build/lib
LIBS = $(LOCAL_LIBS) $(INCLUDE_LOC) $(INCLUDE_SOAPY)
DYN = -lSoapySDR

default: all

all: usb-radio.cpp
	$(CXX) $(LIBS) -g -std=c++17 -c -fPIC usb-radio.cpp -o usb-radio.o
	$(CXX) $(LIBS) $(DYN) -g -std=c++17 usb-radio.o -shared -o usb-radio.so

clean:
	rm -f *.o *.so
