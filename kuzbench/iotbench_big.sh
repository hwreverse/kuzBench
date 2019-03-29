#!/bin/sh
./kbfast --threads 4 --blocksize 256 --loops 4 --verbose >S912_Of_4_256.txt
./kbfast --threads 8 --blocksize 128 --loops 4 --verbose >S912_Of_8_128.txt
./kbfast --threads 16 --blocksize 64 --loops 4 --verbose >S912_Of_16_64.txt
./kbfast --threads 32 --blocksize 32 --loops 4 --verbose >S912_Of_32_32.txt
./kbfast --threads 64 --blocksize 16 --loops 4 --verbose >S912_Of_64_16.txt
./kbfast --threads 128 --blocksize 8 --loops 4 --verbose >S912_Of_128_8.txt
