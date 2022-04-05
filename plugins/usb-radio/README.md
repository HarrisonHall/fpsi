# USB SDR module
Note: this module is not yet complete

## Build or Install SoapySDR
Build by using standard cmake in `include/SoapySDR`:
`mkdir build && cd build && cmake .. && make && sudo make install`.

Arch install 
`sudo pacman -S soapysdr`.

Debian install
`sudo apt install libsoapysdr-dev libsoapysdr0.7`.

## Install SoapyRTLSDR
```bash
cd include/Soapy/RTLSDR
mkdir build
cd build
cmake ..
make
sudo make install
```
