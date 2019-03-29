#!/bin/sh

mkdir -p obj

flags='-O3 -Wall'

#BUILD

gcc $flags -std=c++11  -c crypt_fn.cpp -o obj/crypt_fn.o
gcc $flags -std=c++11  -c main.cpp -o obj/main.o

gcc $flags -c nettle/kuznyechik-meta.c -o obj/kuznyechik-meta.o
gcc $flags -c nettle/nettle-internal.c -o obj/nettle-internal.o
gcc $flags -c nettle/cbc.c -o obj/cbc.o
gcc $flags -c nettle/kuznyechik.c -o obj/kuznyechik.o

#memxor c realization (ARMv8)
gcc $flags -c nettle/memxor.c -o obj/memxor.o
gcc $flags -c nettle/memxor3.c -o obj/memxor3.o

mkdir -p bin

# LINKING
g++ $flags -pthread -lstdc++ -o bin/kuzbench obj/crypt_fn.o obj/main.o obj/kuznyechik-meta.o obj/memxor3.o obj/nettle-internal.o obj/cbc.o obj/memxor.o obj/kuznyechik.o

#RUN
bin/kuzbench --help


