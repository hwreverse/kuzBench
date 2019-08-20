#!/bin/sh

mkdir -p objf

flags='-Ofast -flto -Wall'

#BUILD

gcc $flags -std=c++11  -c crypt_fn.cpp -o objf/crypt_fn.o
gcc $flags -std=c++11  -c main.cpp -o objf/main.o

gcc $flags -c nettle/kuznyechik-meta.c -o objf/kuznyechik-meta.o
gcc $flags -c nettle/nettle-internal.c -o objf/nettle-internal.o
gcc $flags -c nettle/cbc.c -o objf/cbc.o
gcc $flags -c nettle/kuznyechik.c -o objf/kuznyechik.o

#memxor c realization (ARMv8)
gcc $flags -c nettle/memxor.c -o objf/memxor.o
gcc $flags -c nettle/memxor3.c -o objf/memxor3.o

mkdir -p bin

# LINKING
g++ $flags -pthread -lstdc++ -o bin/kbfast objf/crypt_fn.o objf/main.o objf/kuznyechik-meta.o objf/memxor3.o objf/nettle-internal.o objf/cbc.o objf/memxor.o objf/kuznyechik.o

#RUN
bin/kbfast --help


