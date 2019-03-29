#ifndef CRYPT_FN_H
#define CRYPT_FN_H
#include <cstdint>
#include <chrono>
#include <iostream>

#define DEBUG_OUTPUT
#ifdef DEBUG_OUTPUT 
#define D(x) x
#else 
#define D(x)
#endif


#define MAX_THREADS 2048
#define KUZNYECHIK_KEY_SIZE 32


using namespace std;
using namespace chrono;

struct bench_average
{
	int cryptTimePerThread[MAX_THREADS];
	int decryptTimePerThread[MAX_THREADS];
};




/* Set [i] = floor(sqrt(i)) */
uint8_t* init_data(int sizeInMb, int& newArraySize);

uint8_t* init_key();

void cryptThread(bool* waitFlag, int algorithm, int threadNum, uint8_t *inData, uint8_t *outData, int length, bench_average* benchParam);

void decryptThread(bool* waitFlag, int algorithm, int threadNum, uint8_t *inData, uint8_t *outData, int length, bench_average* benchParam);




#endif // CRYPT_FN_H
