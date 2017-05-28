#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

int *tab;
int N;
sem_t lock;

void a(int *tab){
  int i;
  for (i=0; i<N; i++) {
    if (tab[i] == 1) {
      printf("%d ",i);
    }
  }
  printf("\n");
}

void erase_multiple(int m) {
  int k;
  for (k=m+1; k<N; k++) {
    if (k%m == 0) {
      tab[k] = 0;
    }
  }
}

void* slave(void* p) {
  int *id = (int*)p;
  //printf("%d\n",*id);
  erase_multiple(*id);
  sem_post(&lock);
  return NULL;
}

int main(int argc, char* argv[]) {
  int i;
  int k = 0;
  int x = 2;
  struct timespec t0, t1;
  
  // initialisation
  
  N = atoi(argv[1]);
  tab = malloc(N*sizeof(int));
  for (i=2; i<N; i++) {
    tab[i] = 1;
  }
  sem_init(&lock,0,1);

  // travail;
  
  pthread_t t;

  clock_gettime(CLOCK_MONOTONIC, &t0);
  
  while (k < N) {
    sem_wait(&lock);
    do {k++;}while (tab[k] == 0);
    x = k;
    pthread_create(&t,NULL,slave,&x);
    pthread_join(t,NULL);
  }

  a(tab);
  
  clock_gettime(CLOCK_MONOTONIC, &t1);
  
  printf("Done in %.2lfs\n",
	 ((double)t1.tv_sec - t0.tv_sec) +
	 ((double)t1.tv_nsec - t0.tv_nsec) / 1.0E9);
  
  return 0;
}
