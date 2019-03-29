#include "crypt_fn.h"
#include <thread>
#include <stdio.h>
#include "stdlib.h"
#include <cstring>
#include <mutex>
#include "nettle/nettle-meta.h"
#include "nettle/nettle-internal.h"
#include "nettle/nettle-types.h"
#include "nettle/cbc.h"
#include "rpi3arm/rpi3arm.h"

#define KUZNYECHIK_KEY_SIZE 32
#define KUZNYECHIK_SUBKEYS_SIZE (16 * 10)
#define KUZNYECHIK_BLOCK_SIZE 16


mutex mtx;

struct bench_cbc_info
{
	void *ctx;
	nettle_cipher_func *crypt;

	const uint8_t *src;
	uint8_t *dst;

	unsigned block_size;
	uint8_t *iv;
};


void origin_nettle_kuznec_crypt(uint8_t* key, uint8_t *inData, uint8_t *outData, int length)
{

	void * ctx = malloc(nettle_kuznyechik.context_size);
	//uint8_t * key = new uint8_t[nettle_kuznyechik.key_size];

	uint8_t *iv = new uint8_t[KUZNYECHIK_BLOCK_SIZE];

	struct bench_cbc_info info;
	info.ctx = ctx;
	info.crypt = nettle_kuznyechik.encrypt;
	info.src = inData;
	info.dst = outData;
	info.block_size = KUZNYECHIK_BLOCK_SIZE;
	info.iv = iv;
	nettle_kuznyechik.set_encrypt_key(ctx, key);
	cbc_encrypt(ctx, info.crypt,
		info.block_size, info.iv,
		length, info.dst, info.src);
}

void origin_nettle_kuznec_decrypt(uint8_t* key, uint8_t *inData, uint8_t *outData, int length)
{
	void * ctx = malloc(nettle_kuznyechik.context_size);
	//uint8_t * key = new uint8_t[nettle_kuznyechik.key_size];

	uint8_t *iv = new uint8_t[KUZNYECHIK_BLOCK_SIZE];

	struct bench_cbc_info info;
	info.ctx = ctx;
	info.crypt = nettle_kuznyechik.decrypt;
	info.src = inData;
	info.dst = outData;
	info.block_size = KUZNYECHIK_BLOCK_SIZE;
	info.iv = iv;
	nettle_kuznyechik.set_encrypt_key(ctx, key);
	cbc_decrypt(ctx, info.crypt,
		info.block_size, info.iv,
		length, info.dst, info.src);
}

///////////////////////////

#ifdef RPI3ARM
void rpi3arm_kuznec_crypt(uint8_t* key, uint8_t *inData, uint8_t *outData, int length)
{

	void * ctx = malloc(nettle_kuznyechik.context_size);
	//uint8_t * key = new uint8_t[nettle_kuznyechik.key_size];
		
	uint8_t *iv = new uint8_t[KUZNYECHIK_BLOCK_SIZE];

	struct bench_cbc_info info;
	info.ctx = ctx;
	info.crypt = (nettle_cipher_func *)gost_kuznec_encrypt;
	info.src = inData;
	info.dst = outData;
	info.block_size = KUZNYECHIK_BLOCK_SIZE;
	info.iv = iv;
	((nettle_set_key_func *)gost_kuznec_set_key)(ctx, key);
	cbc_encrypt(ctx, info.crypt,
		info.block_size, info.iv,
		length, info.dst, info.src);
}

void rpi3arm_kuznec_decrypt(uint8_t* key, uint8_t *inData, uint8_t *outData, int length)
{
	void * ctx = malloc(nettle_kuznyechik.context_size);
	//uint8_t * key = new uint8_t[nettle_kuznyechik.key_size];

	uint8_t *iv = new uint8_t[KUZNYECHIK_BLOCK_SIZE];

	struct bench_cbc_info info;
	info.ctx = ctx;
	info.crypt = (nettle_cipher_func *)gost_kuznec_decrypt;
	info.src = inData;
	info.dst = outData;
	info.block_size = KUZNYECHIK_BLOCK_SIZE;
	info.iv = iv;
	((nettle_set_key_func *)gost_kuznec_set_key)(ctx, key);
	cbc_decrypt(ctx, info.crypt,
		info.block_size, info.iv,
		length, info.dst, info.src);
}
#endif

////////////////////////////

void cryptThread(bool* waitFlag, int algorithm, int threadNum, uint8_t *inData, uint8_t *outData, int length, bench_average* benchParam)
{

	uint8_t* key = init_key();
	while (*waitFlag)
		this_thread::sleep_for(chrono::microseconds (300));
	auto start = high_resolution_clock::now();
	
	switch (algorithm)
	{
	case 1:
		origin_nettle_kuznec_crypt(key, inData, outData, length);
		break;
	case 2:
#ifdef RPI3ARM
		rpi3arm_kuznec_crypt(key, inData, outData, length);
#endif
		break;
	default:
		throw;
	}

	auto end = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(end - start).count();
	float seconds = duration / 1000.0;
	int size = length * sizeof(uint8_t);
	float speed = (size / 1024.0 / 1024.0) / seconds;
	mtx.lock();
	cout << "crypt thread #" << threadNum << " ended, " 
		<< "time = "  << duration << " ms (" 
		<< seconds << " s)"	<< ", speed = " 
		<< speed << " MB/s (size: " << size/1024/1024
		<< " MB)" << endl;
	mtx.unlock();
	benchParam->cryptTimePerThread[threadNum] = duration;
}

void decryptThread(bool* waitFlag, int algorithm, int threadNum, uint8_t *inData, uint8_t *outData, int length, bench_average* benchParam)
{
	uint8_t* key = init_key();
	while (*waitFlag)
		this_thread::sleep_for(chrono::milliseconds(3));
	auto start = high_resolution_clock::now();
	
	switch (algorithm)
	{
	case 1:
		origin_nettle_kuznec_decrypt(key, inData, outData, length);
		break;
	case 2:
#ifdef RPI3ARM
		rpi3arm_kuznec_decrypt(key, inData, outData, length);
#endif		
		break;
	default:
		throw;
	}

	auto end = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(end - start).count();
	float seconds = duration / 1000.0;
	int size = length * sizeof(uint8_t);
	float speed = (size / 1024.0 / 1024.0) / seconds;
	mtx.lock();
	cout << "decrypt thread #" << threadNum << " ended, "
		<< "time = " << duration << " ms ("
		<< seconds << " s)" << ", speed = "
		<< speed << " MB/s (size: " << size / 1024 / 1024
		<< " MB)" << endl;
	mtx.unlock();
	benchParam->decryptTimePerThread[threadNum] = duration;
	
}

////////////////////////////
/* Set [i] = floor(sqrt(i)) */
uint8_t* init_data(int sizeInMb, int& newArraySize)
{
	int arrLength = (sizeInMb*1024*1024) / sizeof(uint8_t);
	uint8_t * result = new uint8_t[arrLength];
	unsigned i, j;
	for (i = j = 0; i<arrLength; i++)
	{
		if (j*j < i)
			j++;
		result[i] = j;
	}
	newArraySize = arrLength;
	return result;
}

uint8_t* init_key()
{
	uint8_t* result = new uint8_t[KUZNYECHIK_KEY_SIZE];
	unsigned i;
	for (i = 0; i<KUZNYECHIK_KEY_SIZE; i++)
		result[i] = i;
	return result;
}




