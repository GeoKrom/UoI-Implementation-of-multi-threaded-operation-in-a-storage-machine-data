#include <string.h>
#include "../engine/db.h"
#include "../engine/variant.h"
#include "bench.h"

#define DATAS ("testdb")

DB* db;

void handleDB(char* action){ //Synarthsh gia anoigma-kleisimo vasis. Thn kanoyme epeidh theloume na kanoume energeies aytes sto arxeio bench.c
	if(strcmp("open",action) == 0){
		db = db_open(DATAS);
	}
	if(strcmp("close",action) == 0){
		db_close(db);
	}
}

void _write_test(long int count, int r, int threads) //Prosthetoume to orisma "threads". Edw tha diamoirasoume poses leitoyrgies tha ginoun ana thread.
{	
	int i;
	double cost;
	long long start,end;
	Variant sk, sv;
	long int split; //Metavliti pou tha krataei posa writes/trhead
	//DB* db;

	char key[KSIZE + 1];
	char val[VSIZE + 1];
	char sbuf[1024];

	memset(key, 0, KSIZE + 1);
	memset(val, 0, VSIZE + 1);
	memset(sbuf, 0, 1024);

	//db = db_open(DATAS); //Syanrthsh ths engine sto db.c-anoigma vashs
	
	start = get_ustime_sec();
	split = count/threads; // Leme posa requests na ginoun ana thread.
	for (i = 0; i < split; i++) { //Allazoume to hdh yparxon "count". H eggrafh na treksei gia ta sygkekrimena requests.
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d adding %s\n", i, key); // Print pou ginetai me thn ektelesh tou paradeigmatos ekfwnhshs
		snprintf(val, VSIZE, "val-%d", i);

		sk.length = KSIZE;
		sk.mem = key;
		sv.length = VSIZE;
		sv.mem = val;

		db_add(db, &sk, &sv);
		if ((i % 10000) == 0) {
			fprintf(stderr,"random write finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}

	//db_close(db); //Syanrthsh ths engine sto db.c-kleisimo vashs

	end = get_ustime_sec();
	cost = end -start;
	
	pthread_mutex_lock(&numwrites);
	
	writescost = writescost + cost;
	
	pthread_mutex_unlock(&numwrites);


	/*printf(LINE);
	printf("|Random-Write	(done:%ld): %.6f sec/op; %.1f writes/sec(estimated); cost:%.3f(sec);\n"
		,count, (double)(cost / count)
		,(double)(count / cost)
		,cost);*/	
}

void _read_test(long int count, int r, int threads) // Antistoixa prosthetoume to "threads".
{
	int i;
	int ret;
	int found = 0;
	double cost;
	long long start,end;
	Variant sk;
	Variant sv;
	long int split; //Metavliti pou tha krataei posa reads/trhead
	
	//DB* db;
	char key[KSIZE + 1];

	//db = db_open(DATAS); //Syanrthsh ths engine sto db.c-anoigma vashs

	start = get_ustime_sec();
	split = count/threads; // Leme posa requests na ginoun ana thread.
	for (i = 0; i < split; i++) {
		memset(key, 0, KSIZE + 1);

		/* if you want to test random write, use the following */
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d searching %s\n", i, key); // Print pou ginetai me thn ektelesh tou paradeigmatos ekfwnhshs
		sk.length = KSIZE;
		sk.mem = key;
		ret = db_get(db, &sk, &sv); 
		if (ret) {
			//db_free_data(sv.mem);
			found++;
		} else {
			INFO("not found key#%s", 
					sk.mem);
    	}

		if ((i % 10000) == 0) {
			fprintf(stderr,"random read finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}

	//db_close(db); //Syanrthsh ths engine sto db.c-kleisimo vashs

	end = get_ustime_sec();
	cost = end - start;
	pthread_mutex_lock(&numreads);
	readscost = readscost + cost;
	pthread_mutex_unlock(&numreads);
	
	//Kanoume dika mas prints, den xreiazontai ayta.
	/*printf(LINE);
	printf("|Random-Read	(done:%ld, found:%d): %.6f sec/op; %.1f reads /sec(estimated); cost:%.3f(sec)\n",
		count, found,
		(double)(cost / count),
		(double)(count / cost),
		cost); */
}
