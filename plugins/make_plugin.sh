#!/bin/bash
g++ -c -fPIC $1 -o "$1".o
g++ "$1".o -shared -o "$1".so
rm -f "$1".o
