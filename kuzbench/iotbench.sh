#!/bin/sh

./kuzbench --threads 16 --blocksize 32 --loops 4 --verbose >UA_O3_16_32.txt
./kuzbench --threads 8 --blocksize 64 --loops 4 --verbose >UA_O3_8_64.txt
./kbfast --threads 16 --blocksize 32 --loops 4 --verbose >UA_Of_16_32.txt
./kbfast --threads 8 --blocksize 64 --loops 4 --verbose >UA_Of_8_64.txt
