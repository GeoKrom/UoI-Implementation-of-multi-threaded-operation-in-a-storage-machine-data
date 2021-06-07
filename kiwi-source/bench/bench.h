#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define KSIZE (16)
#define VSIZE (1000)

#define LINE "+-----------------------------+----------------+------------------------------+-------------------+\n"
#define LINE1 "---------------------------------------------------------------------------------------------------\n"

long long get_ustime_sec(void);
void _random_key(char *key,int length);

//Struct: To xreiazomaste efoson theloume na kratame idia stoixeia
//(count = arithmos requests, threads = nimata pou tha dwsoume, r = to r pou tha dwsthei apo to programma apla gia na pernaei san orisma sthn read h write pou tha kaleitai apo thn kiwi.c)
//gia 3 leitoyrgies (write/read/readwrite)
struct holder
{
	int count_holder;
	int thread_holder;
	int r_holder;
};

pthread_mutex_t numwrites; // Dhlwsh twn metavlitwn gia na krathsoume ta kosth
pthread_mutex_t numreads;
double writescost;
double readscost;