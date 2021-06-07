#include "bench.h"

void _random_key(char *key,int length) {
	int i;
	char salt[36]= "abcdefghijklmnopqrstuvwxyz0123456789";

	for (i = 0; i < length; i++)
		key[i] = salt[rand() % 36];
}

void _print_header(int count)
{
	double index_size = (double)((double)(KSIZE + 8 + 1) * count) / 1048576.0;
	double data_size = (double)((double)(VSIZE + 4) * count) / 1048576.0;

	printf("Keys:\t\t%d bytes each\n", 
			KSIZE);
	printf("Values: \t%d bytes each\n", 
			VSIZE);
	printf("Entries:\t%d\n", 
			count);
	printf("IndexSize:\t%.1f MB (estimated)\n",
			index_size);
	printf("DataSize:\t%.1f MB (estimated)\n",
			data_size);

	printf(LINE1);
}

void _print_environment()
{
	time_t now = time(NULL);

	printf("Date:\t\t%s", 
			(char*)ctime(&now));

	int num_cpus = 0;
	char cpu_type[256] = {0};
	char cache_size[256] = {0};

	FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
	if (cpuinfo) {
		char line[1024] = {0};
		while (fgets(line, sizeof(line), cpuinfo) != NULL) {
			const char* sep = strchr(line, ':');
			if (sep == NULL || strlen(sep) < 10)
				continue;

			char key[1024] = {0};
			char val[1024] = {0};
			strncpy(key, line, sep-1-line);
			strncpy(val, sep+1, strlen(sep)-1);
			if (strcmp("model name", key) == 0) {
				num_cpus++;
				strcpy(cpu_type, val);
			}
			else if (strcmp("cache size", key) == 0)
				strncpy(cache_size, val + 1, strlen(val) - 1);	
		}

		fclose(cpuinfo);
		printf("CPU:\t\t%d * %s", 
				num_cpus, 
				cpu_type);

		printf("CPUCache:\t%s\n", 
				cache_size);
	}
}

///////WRITERS AND READERS CALLED /////////
void *request_handler_w(void *arg) // Genikh voithitiki synarthsh gia ton xeirismo twn zhtoumenwn write h readmesa sth main. 
{
	struct holder* rh = (struct holder *)arg; //Arxikopoihsh mias metavliths gia thn diaxeirhsh tou struct px. rh apo to request handler
	_write_test(rh->count_holder, rh->r_holder, rh->thread_holder); //tote kalese thn _write_test apo to kiwi.c arxeio
	return 0;
}

void *request_handler_r(void *arg){
	struct holder* rh = (struct holder *)arg;
	_read_test(rh->count_holder, rh->r_holder, rh->thread_holder); //kalese thn _read_test
	return 0;
}
//////////////////////////////////////////

//// CALCULATE AND PRINT RESULTS//////////

void *printer(char* action, int count, double all){
	if(strcmp("write",action) == 0){
		printf("WRITERS statistics: \n Number of requests: %.d \n Req. service time: %3f \n Writes per second: %.f \n Total Cost: %.3f sec \n", count, (double)(all/count), (double)(count/all), all);
	}
	if(strcmp("read",action) == 0){
		printf("READERS statistics: \n Number of requests: %.d \n Req. service time: %3f \n Reads per second: %.f \n Total Cost: %.3f sec \n", count,(double)(all/count), (double)(count/all), all);
	}
	return 0;
}
/////////////////////////////////////////

int main(int argc,char** argv)
{
	long int count;
	int numot = atoi(argv[3]);	//Number Of Threads - Syntomia "numot" pou tha dinontai san orisma.
	
	pthread_t *genthreads, *wthreads, *rthreads;
	
	genthreads = (pthread_t*)malloc(numot*sizeof(pthread_t)); //general threads: Gia tis 2 protes if read/write. Mia metavliti ftanei efoson tha bei sthn mia h thn allh periptwsh.
	wthreads = (pthread_t*)malloc(numot*sizeof(pthread_t)); //Ayth kai h epomenh tha xreiastoun gia to IF ths readwrite, efoson theloume na kratame taytoxrona reads kai writes prepei na diaxwrisoume tis leitoyrgies.
	rthreads = (pthread_t*)malloc(numot*sizeof(pthread_t)); //Ta kaname eksarxis malloc sto megethos pou tha dwthei gia eykolia.Epishs, efoson tha doulepsoun posostiaia den tha to kseperasoun.
	
	struct holder general,writers,readers; //Tha kratame se kathe if pou mporei na bei o kwdikas ta current stoixeia (count,threads,r) sto struct.
	
	pthread_mutex_init(&numwrites, NULL); //Arxikopoihsh
	pthread_mutex_init(&numreads, NULL);
	writescost = 0;
	readscost = 0;
	
	srand(time(NULL));
	if (argc < 4 && ((strcmp(argv[1], "write") == 0)|| (strcmp(argv[1], "read") == 0))) { //thelw na dinw extra orisma ta nimata, sthn 3i thesi (argv[3]). Opote ayksanw ton arithmo ston elegxo kata 1.
		fprintf(stderr,"Usage: db-bench <write / read / readwrite> <count> <num_of_threads> <r> \n");
		exit(1);
	}
	
	if (argc < 6 && (strcmp(argv[1], "readwrite") == 0)) { //thelw na dinw extra orisma ta nimata, sthn 3i thesi (argv[3]). Opote ayksanw ton arithmo ston elegxo kata 1.
		fprintf(stderr,"Usage: db-bench <write / read / readwrite> <count> <num_of_threads> <write_percentage> <read_percentage> <r>\n");
		exit(1);
	}
	
	
	if (strcmp(argv[1], "write") == 0) {
		int r = 0;
		count = atoi(argv[2]); //Orisma gia arithmo aithmatwn
		_print_header(count);
		_print_environment();
		
		if (argc == 5) //Kai edw ayksisi tou arithmou synthikis efoson prosthesame allo 1 orisma(nimata), dhladh 5 mazi me to r.
			r = 1;

		handleDB("open"); //Anoigei h vasi gia write.
		
		//apothikeyoume sto struct tis prosorines times twn count,threads,r
		general.count_holder = count;
		general.thread_holder = numot;
		general.r_holder = r;
		
		//Twra theloume na dhmioyrghsoume ta nimata pou tha epitelesoun thn leitoyrgia tou writing
		int i,j = 0;
		while(i<numot){
			pthread_create(&genthreads[i], NULL, request_handler_w, (void*)&general);
			i++;
		}
		
		while(j<numot){
			pthread_join(genthreads[j], NULL);
			j++;
		}
		
		handleDB("close"); // Kleisimo vasis.
		
		//Ektypwsh apotelesmatwn
		printer("write",general.count_holder,writescost);
		
	} else if (strcmp(argv[1], "read") == 0) {
		int r = 0;
		count = atoi(argv[2]); //Orisma gia arithmo aithmatwn
		_print_header(count);
		_print_environment();
		
		if (argc == 5) //Kai edw ayksisi tou arithmou synthikis efoson prosthesame allo 1 orisma(nimata), dhladh 5 mazi me to r.
			r = 1;


		handleDB("open"); //Anoigei h vasi gia write.
		
		//apothikeyoume sto struct tis prosorines times twn count,threads,r
		general.count_holder = count;
		general.thread_holder = numot;
		general.r_holder = r;
		
		//Twra theloume na dhmioyrghsoume ta nimata pou tha epitelesoun thn leitoyrgia tou writing
		int i = 0; int j = 0;
		while(i<numot){
			pthread_create(&genthreads[i], NULL, request_handler_r, (void*)&general);
			i++;
		}
		
		while(j<numot){
			pthread_join(genthreads[j], NULL);
			j++;
		}
		
		handleDB("close"); // Kleisimo vasis.
		
		//Ektypwsh apotelesmatwn
		printer("read",general.count_holder,readscost);
		
	} else if (strcmp(argv[1], "readwrite") == 0) {
		int r = 0;
		count  = atoi(argv[2]);
		_print_header(count);
		_print_environment();
		if(argc == 7){
			r = 1;
		}
		
		double wper = atoi(argv[4]); //Pososto pou theloume gia PUT (write percentage)
		wper = wper / 100;
		double rper = atoi(argv[5]); //Pososto pou theloume gia GET (read percentage)
		rper = rper / 100;
		
		handleDB("open");
		
		//Arxikopoihsh ths domhs gia readers
		readers.count_holder = (long)(count*rper);
		readers.thread_holder = (int)(numot*rper);
		readers.r_holder = r;
		
		//Dhmiourgia nimatwn readers.
		int k = 0; int l = 0;
		while(k<numot*rper){
			pthread_create(&rthreads[k], NULL, request_handler_r, (void*)&readers);
			k++;
		}
		
		while(l<numot*rper){
			pthread_join(rthreads[l], NULL);
			l++;
		}
		
		//Arxikopoihsh ths domhs gia writers
		writers.count_holder = (long)(count*wper);
		writers.thread_holder = (int)(numot*wper);
		writers.r_holder = r;
		
		//Dhmiourgia nhmatwn writers.
		int i = 0; int j = 0;
		while(i<numot*wper){
			pthread_create(&wthreads[i], NULL, request_handler_w, (void*)&writers);
			i++;
		}
		
		while(j<numot*wper){
			pthread_join(wthreads[j], NULL);
			j++;
		}
		
		handleDB("close");
		
		//Ektypwsh apotelesmatwn.
		printer("write",writers.count_holder,writescost);
		printer("read",readers.count_holder,readscost);
		
	} else {
		fprintf(stderr,"Usage: db-bench <write / read / readwrite> <count> <num_of_threads> <r> \n");
		exit(1);
	}

	free(genthreads); //Apodesmeysi twn malloc
	free(wthreads);
	free(rthreads);
	return 1;
}
