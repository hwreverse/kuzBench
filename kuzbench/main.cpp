//kuzbench 1.5 Threadpool=2048
//Author: alxm (2018)

#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include "crypt_fn.h"
#include <iostream>
#include <cstring>



using namespace std;
using namespace chrono;



void printHelp()
{
	cout << endl << "Available flags:" << endl
		<< "\t--threads \t Thread count" << endl
		<< "\t--blocksize \t Block size for one thread in megabytes" << endl
		<< "\t--loops \t Repeate test count (for average values) " << endl
		<< "\t--alg \t Algorithm number. 1 - nettle, 2 - deprecated" << endl
		<< "\t--verbose \t Output additional info (lscpu)" << endl
		<< "\t--help -h \t This help " << endl
		<< endl << "Example: kuzbench --threads 4 --blocksize 32 --loops 1 --alg 1 --verbose" << endl;
}

int strToInt(char* numStr)
{
	istringstream ssStr(numStr);
	int result;
	if (ssStr >> result)
		return result;
	//wrong
	cerr << " Wrong number! " << numStr << endl;
	printHelp();
	exit(1);
	
}
void setBenchParams(int cnt, char* params[], int& blockSizeMb, int& threadCnt, int& loopCnt, int& alg, int& verbose)
{
	threadCnt = 4;
	blockSizeMb = 512 / threadCnt * 2 + 1;
	loopCnt = 1;
	verbose = 0;
	alg = 1;
	for (int i = 1; i < cnt; ++i) {


		if (string(params[i]) == "--threads" && i + 1 < cnt)
			threadCnt = strToInt(params[++i]);
		else if (string(params[i]) == "--blocksize" && i + 1 < cnt)
			blockSizeMb = strToInt(params[++i]);
		else if (string(params[i]) == "--loops" && i + 1 < cnt)
			loopCnt = strToInt(params[++i]);
		else if (string(params[i]) == "--alg" && i + 1 < cnt)
			alg = strToInt(params[++i]);
		else if (string(params[i]) == "--verbose")
			verbose = 1;
		else if (string(params[i]) == "--help" || string(params[i]) == "-h")
		{
			printHelp();
			exit(1);
		}
		else {
			cerr << " Wrong flags! " << endl;
			printHelp();
			exit(1);
		}
	}

	if (alg > 2)
	{
		cerr << " Wrong flags! " << endl;
		printHelp();
		exit(1);
	}

	printf("Threads: %d, block: %d MB (mem: %d MB)\n",
		threadCnt, blockSizeMb, blockSizeMb + threadCnt * blockSizeMb * 2);
	printf("Loops: %d, algorithm: %s \n",
		loopCnt, ((alg == 1) ? "1(nettle)" : "2(rpi3arm)"));
	
}


int main(int argc, char* argv[])
{
	
	cout << "kuzbench 1.5 - 2048T" << endl << endl;
	int blockSize, threadCnt, loopCnt, alg, verbose;
	setBenchParams(argc, argv, blockSize, threadCnt, loopCnt, alg, verbose);
	if (verbose)
	{
		cout << "=======   lscpu  =========" << endl;
		system("lscpu");
		cout << "==/proc/cpuinfo hardware==" << endl;
		system("cat /proc/cpuinfo | grep Hardware");
		cout << "==========================" << endl;
	}
	int maxThreads = MAX_THREADS;
	if (threadCnt > maxThreads)
	{
		cerr << "Max threads is " << maxThreads << endl;
		exit(1);
	}
	//init data arrays
	u_int8_t *sourceData = nullptr;
	int dataLength = 0;
	thread pool[maxThreads];
	u_int8_t* cryptedData[maxThreads];
	u_int8_t* decryptedData[maxThreads];
	int currentAllocMem = 0;
	try
	{
		sourceData = init_data(blockSize, dataLength);
		currentAllocMem += blockSize;
		for (int iT = 0; iT < threadCnt; iT++)
		{
			cryptedData[iT] = new u_int8_t[dataLength];
			memset(cryptedData[iT], 0, dataLength);
			currentAllocMem += blockSize;
			decryptedData[iT] = new u_int8_t[dataLength];
			memset(decryptedData[iT], 0, dataLength);
			currentAllocMem += blockSize;
		}
	}
	catch (std::bad_alloc& ba)
	{

		cerr << "Not enough memory, try decrease block size. " 
			<< ba.what() << endl;
		cerr << "Current alloc. : " << currentAllocMem << " MB"
			<< endl;

		exit(1);
	}
	
	auto benchResults = new bench_average[loopCnt];

	//Start bench
	auto start = high_resolution_clock::now();
	high_resolution_clock::time_point startCreateThreads, stopCreateThreads, startCrypt, stopCrypt, startDecrypt, stopDecrypt;
	bool *wait = new bool;
	for (int iL = 0; iL < loopCnt; iL++)
	{
		if (loopCnt > 1)
			cout << "Cycle #" << iL + 1 << endl;
		
		cout << "Start crypt thread(s)..." << endl;
		*wait = true;
		startCreateThreads = high_resolution_clock::now();
		for (int iT = 0; iT < threadCnt; iT++)
		{
			pool[iT] = thread(cryptThread, wait, alg, iT, sourceData, cryptedData[iT], dataLength, &benchResults[iL]);
		}
		stopCreateThreads = high_resolution_clock::now();
		
		*wait = false;
		startCrypt = high_resolution_clock::now();
		for (int iT = 0; iT < threadCnt; iT++)
		{
			pool[iT].join();
		}
		stopCrypt = high_resolution_clock::now();
		cout << "Start DEcrypt thread(s)..." << endl;
		*wait = true;
		for (int iT = 0; iT < threadCnt; iT++)
		{
			pool[iT] = thread(decryptThread, wait, alg, iT, cryptedData[iT], decryptedData[iT], dataLength, &benchResults[iL]);
		}
		
		*wait = false;
		startDecrypt = high_resolution_clock::now();
		for (int iT = 0; iT < threadCnt; iT++)
		{
			pool[iT].join();
		}
		stopDecrypt = high_resolution_clock::now();
	}
	auto end = high_resolution_clock::now();
	
	auto duration = duration_cast<milliseconds>(end - start).count();
	auto createThreadsTime = duration_cast<milliseconds>(stopCreateThreads - startCreateThreads).count();
	auto totalCryptTime = duration_cast<milliseconds>(stopCrypt - startCrypt).count();
	auto totalDecryptTime = duration_cast<milliseconds>(stopDecrypt - startDecrypt).count();
	//Summary output
	cout << "Summary (for all threads):" << endl;
	printf("\tData size (block x threads): \t %d (%d MB x %d)\n", 
		blockSize * threadCnt, blockSize, threadCnt);
	printf("\tMem alloc(plain(1) + \n\t crypt(%d) + decrypt(%d)): \t %d MB\n", 
		threadCnt, threadCnt, blockSize + threadCnt * blockSize * 2);

	printf("\tCreate %d threads time: \t %f s \n", threadCnt, (createThreadsTime/1000.0));
	printf("\tCrypt time: \t\t\t %f s \n", (totalCryptTime/1000.0));
	printf("\tDecrypt time: \t\t\t %f s \n", (totalDecryptTime/1000.0));
	printf("\tTotal work time: \t\t %f s \n", (duration/1000.0));
	
	int avgCryptTime = 0; 
	int avgDecryptTime = 0;
	for (int iL = 0; iL < loopCnt; iL++)
	{
		for (int iT = 0; iT < threadCnt; iT++)
		{
			avgCryptTime += benchResults[iL].cryptTimePerThread[iT];
			avgDecryptTime+= benchResults[iL].decryptTimePerThread[iT];
		}
	}
	avgCryptTime = avgCryptTime / (loopCnt*threadCnt);
	avgDecryptTime = avgDecryptTime / (loopCnt*threadCnt);
	printf("\t Avg. crypt time:   \t%d ms (%f s)\n",
		avgCryptTime, avgCryptTime / 1000.0);
	printf("\t Avg. decrypt. time:   \t%d ms (%f s)\n",
		avgDecryptTime, avgDecryptTime / 1000.0);
	printf("\t Avg. encrypt. speed:   \t%f  MB/s\n",
		(blockSize * threadCnt) / (avgCryptTime / 1000.0));
	printf("\t Avg. decrypt. speed:   \t%f  MB/s\n",
		(blockSize * threadCnt) / (avgDecryptTime / 1000.0));
	
	if (verbose)
	{
		cout << endl <<  "Crypt result info: " << endl;
		int symbolCnt = 30;
		cout << "First " << symbolCnt << " symbols for:" << endl;
		cout << "source data:" << endl;
		for (int i = 0; i < symbolCnt; i++)
			printf("%02x ",sourceData[i]);
		cout << endl << "encrypted data:" << endl;
		for (int i = 0; i < symbolCnt; i++)
			printf("%02x ", cryptedData[0][i]);
		cout << endl << "decrypted data:" << endl;
		for (int i = 0; i < symbolCnt; i++)
			printf("%02x ", decryptedData[0][i]);
		cout << endl;
	}

	return 0;
}