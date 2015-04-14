#! /bin/sh
g++ -c cmap_conv.cpp -o obj/cmap_conv.o
g++ -c stream.cpp -o obj/stream.o
g++ -o bin/Release/cmap_conv obj/cmap_conv.o obj/stream.o -lpng -lz
