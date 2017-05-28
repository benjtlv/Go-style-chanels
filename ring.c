#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <string.h>
#include <time.h>
#include "channel.h"

#define N 1000
#define LIM 10

int write_file(int fd,char *buf){
  int i;
  for (i = 0; i < LIM; i++) {
    if (write(fd,buf,strlen(buf)) == -1){
      perror("write ");
      return -1;
    }
  }
  return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
  int i;
  int fd;
  int tab[N][2];
  struct channel* channels[N];
  struct timespec t0, t1;
  char *buf = "jeton";
  int jeton = 1;
  char *num = malloc(sizeof(char));

  //N = atoi(argv[1]);
  
  if ((fd = open("fichier",O_CREAT | O_WRONLY ,0644)) == -1) {
    perror("open ");
    return EXIT_FAILURE;
  }
  
  for (i = 0; i < N; i++) {
    if (pipe(tab[i]) == -1) {
      perror("pipe ");
      return EXIT_FAILURE;
    }
    channels[i] = channel_create(sizeof(int),1,CHANNEL_PROCESS_SHARED);
  }

  clock_gettime(CLOCK_MONOTONIC, &t0);

  if (fork() == 0) {
    for (i=0; i<N; i++) {
      if (fork() == 0) {
	switch(i) {
	case 0 :
	   write_file(fd,"0");
	   if (argc == 3 && (!strcmp(argv[2],"-c") > 0)) {
	    channel_send(channels[0],&jeton);
	  } else {
	    close(tab[0][0]);
	    write(tab[0][1],buf,strlen(buf));
	    close(tab[0][1]);
	  }
	  break;
	  
	case N-1 :
	  if (argc == 3 && !strcmp(argv[2],"-c") > 0) {
	    channel_recv(channels[N-2],&jeton);
	  } else {
	    close(tab[N-2][1]);
	    read(tab[N-2][0],buf,strlen(buf));
	    close(tab[N-2][0]);
	  }
	  sprintf(num,"%d",N-1);
	  write_file(fd,num);
	  
	  clock_gettime(CLOCK_MONOTONIC, &t1);
	  
	  printf("Done in %.2lfs\n",
		 ((double)t1.tv_sec - t0.tv_sec) +
		 ((double)t1.tv_nsec - t0.tv_nsec) / 1.0E9);
	  break;
	  
	default :
	  if (argc == 3 && !strcmp(argv[2],"-c") > 0) {
	    channel_recv(channels[i-1],&jeton);
	  } else {
	    close(tab[i-1][1]);
	    read(tab[i-1][0],buf,strlen(buf));
	    close(tab[i-1][0]);
	  }
	  sprintf(num,"%d",i);
	  write_file(fd,num);
	  if (argc == 3 && !strcmp(argv[2],"-c") > 0) {
	    channel_send(channels[i],&jeton);
	  } else {
	    write(tab[i][1],buf,strlen(buf));
	  }
	  break;
	}
	break;
      } else {
	if (i == N-1) exit(0);
      }
    }
  } else {
    wait(NULL);
  }
  return 0;
}
