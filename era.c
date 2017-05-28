#include "channel.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct channel *channel_t;

typedef struct duplex_chans_s
{
	channel_t in;
	channel_t out;
}
duplex_chans_t;
  
/* int workers = 0; */

int primes = 0;

int limit = 1000;
 
int qsize = 2048;

channel_t main_chan;

void* era_worker (void *p)
{
	/* int worker = workers++; */
	int id = -1;
	int n = -1;
	int i;
	duplex_chans_t chans = *(duplex_chans_t *) p;
	do{
		/* printf("waiting for input %d \n", worker); */
		int rc = channel_recv ( chans.in, (void *) &id);
		/* printf("got input or failed%d \n", worker); */
		if (rc == 0 || id == -1)
		{
			channel_destroy (chans.in);
			channel_close (chans.out);
			channel_destroy (chans.out);
			channel_send (main_chan, &id);
			return NULL;
		}
		else if (id == 1)
		{
			for (i = 2; i < limit; i++)
			{
				channel_send (chans.out, (void *) &i);
			}
			i = -1;
			channel_send (chans.out, (void *) &i);
			/* printf(" wokrer %d blebla\n", worker); */
		}
		else
		{
			primes++;
			printf("%d ", id);
			do{
				rc = channel_recv (chans.in, (void *) &n);
				/* printf("received worker:%d id : %d %d\n",worker, id , n ); */
				if (rc == 0)
				{
					perror ("channel_recv");
				}
				if (n % id != 0)
				{
					/* printf ("sent worker:%d id:%d %d\n", worker, id, n); */
					channel_send (chans.out, (void *) &n);
				}
				if (n == -1)
				{
					id = 0;
				}
			} while (id != 0);
		}
	} while (1);
	return NULL;
}

int main(int argc, char *argv[])
{
	int rc, i = 0, numthreads = 0;
	struct timespec begin, end;
	const char *usage = "./era [-l limit] [-n numthreads] [-s queu size]";

	while(1) {
		int opt = getopt(argc, argv, "l:n:s:");
		if(opt < 0)
		break;

		switch(opt) {
			case 'l':
				limit = atoi(optarg);
				break;
			case 'n':
				numthreads = atoi (optarg);
				break;
			case 's':
				qsize = atoi(optarg);
				break;
			default:
				fprintf(stderr, "%s\n", usage);
				exit(1);
		}
	}

	if(optind < argc) {
		fprintf(stderr, "%s\n", usage);
		exit(1);
	}


	if(numthreads <= 0)
		numthreads = sysconf(_SC_NPROCESSORS_ONLN);
	if(numthreads <= 0)
	{
		perror("sysconf(_SC_NPROCESSORS_ONLN)");
		exit(1);
	}
	pthread_t workers [numthreads];
	channel_t channels [numthreads];
	duplex_chans_t duplexes [numthreads];
	
	main_chan = channel_create (sizeof (int), 0, 0);
	
	for (i = 0; i < numthreads; i++)
	{
		channels[i] = channel_create (sizeof (int), qsize, 0);
	}

	for (i = 0; i < numthreads; i++)
	{
		duplexes[i].in = channels[i];

		if (i == numthreads - 1)
		{
			duplexes[i].out = channels[0];
		}
		else
		{
			duplexes[i].out = channels[i+1];
		}

	}

	for (i = 0; i < numthreads; i++)
	{
		pthread_create (&(workers[i]),NULL, era_worker, &(duplexes[i]));
		pthread_detach (workers[i]);
	}

	i = 1;
	printf("Running %d worker threads.\n", numthreads);
	clock_gettime(CLOCK_MONOTONIC, &begin);
	rc = channel_send (channels[0], &i);
	if (rc == 0)
		perror ("channel_send");

	rc = channel_recv (main_chan, &i);

	if (rc == 0)
		perror ("channel_recv");
	  
	clock_gettime(CLOCK_MONOTONIC, &end); 
	printf("\nDone in %.2lfs\n",
			((double)end.tv_sec - begin.tv_sec) +
			((double)end.tv_nsec - begin.tv_nsec) / 1.0E9);


	printf("prime numbers %d\n", primes);
	return 0;
}
