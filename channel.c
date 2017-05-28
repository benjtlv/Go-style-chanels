#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "static_fifo.h"
#include "channel.h"
#include <unistd.h>
#include <semaphore.h>

/** This structure defines a channel.
 * It contains a pointer to a fifo structure and a posix mutex */

typedef struct channel
{
  unsigned int shared;
  size_t eltsize;
  unsigned int open;
  unsigned int size;
  static_fifo_t *fp;
  char* data;
  sem_t* read;
  sem_t* write;
  sem_t* access;
  sem_t* access2;
}
*channel_t;

/** This subroutine allocates and initializes a local channel
 */

struct channel *
sync_channel_create (int eltsize)
{
  channel_t c = (struct channel *) malloc (sizeof (struct channel));

  c->shared = CHANNEL_PROCESS_UNSHARED;

  c->data = malloc(eltsize);
  
  if ( c == NULL && errno == ENOMEM)
    {
      perror ("malloc(channel)");
      return NULL;
    }

  //memset(c->data,0,eltsize);
  
  c->read = (sem_t*) malloc(sizeof(sem_t));

  if ( c->read == NULL && errno == ENOMEM)
    {
      perror("malloc(sempahore read) ");
      return NULL;
    }
  
  c->write = (sem_t*) malloc(sizeof(sem_t));

  if ( c->write == NULL && errno == ENOMEM)
    {
      perror("malloc(sempahore write) ");
      return NULL;
    }
  
  c->access = (sem_t*) malloc(sizeof(sem_t));

  if ( c->access == NULL && errno == ENOMEM)
    {
      perror("malloc(sempahore access) ");
      return NULL;
    }

  c->access2 = (sem_t*) malloc(sizeof(sem_t));

  if ( c->access2 == NULL && errno == ENOMEM)
    {
      perror("malloc(sempahore access2) ");
      return NULL;
    }
  
  c->eltsize = eltsize;
  c->size = 0;
  c->open = CHANNEL_OPEN;
  c->fp = NULL;
  
  if(sem_init(c->read,0,0) == -1){
    perror ("sem_init(read)");
    return NULL;
  }
	
  if(sem_init(c->write,0,0) == -1){
    perror("sem_init(write)");
    return NULL;
  }

  if(sem_init(c->access,0,1) == -1){
    perror("sem_init(access)");
    return NULL;
  }

  if(sem_init(c->access2,0,1) == -1){
    perror("sem_init(access2)");
    return NULL;
  }
  
  return c;
}

struct channel *
async_channel_create (int eltsize, int size)
{
  channel_t c = (struct channel *) malloc (sizeof (struct channel));

  c->shared = CHANNEL_PROCESS_UNSHARED;

  if ( c == NULL && errno == ENOMEM)
    {
      perror ("malloc(channel)");
      return NULL;
    }

  //memset(c->data,0,eltsize);
  
  c->read = (sem_t*) malloc(sizeof(sem_t));

  if ( c->read == NULL && errno == ENOMEM)
    {
      perror("malloc(sempahore read) ");
      return NULL;
    }
  
  c->write = (sem_t*) malloc(sizeof(sem_t));

  if ( c->write == NULL && errno == ENOMEM)
    {
      perror("malloc(sempahore write) ");
      return NULL;
    }
  
  c->access = (sem_t*) malloc(sizeof(sem_t));

  if ( c->access == NULL && errno == ENOMEM)
    {
      perror("malloc(sempahore access) ");
      return NULL;
    }
  
  c->eltsize = eltsize;
  c->size = size;
  c->open = CHANNEL_OPEN;
  c->fp = static_new_fifo (size, eltsize, 0, 0);

  if(c->fp == NULL){
    perror("static_new_fifo ");
    return NULL;
  }
  
  if(sem_init(c->read,0,0) == -1){
    perror ("sem_init(read)");
    return NULL;
  }
	
  if(sem_init(c->write,0,size) == -1){
    perror("sem_init(write)");
    return NULL;
  }

  if(sem_init(c->access,0,1) == -1){
    perror("sem_init(access)");
    return NULL;
  }

  if (c->fp == NULL)
    {
      perror ("new_fifo()");
      return NULL;
    }

  return c;
}

struct channel *
local_channel_create (int eltsize, int size)
{
  if (size > 0) {
    return async_channel_create(eltsize,size);
  } else {
    return sync_channel_create(eltsize);
  }
}

/** This subr
outine allocates and initializes a global channel
 */

struct channel *
global_channel_create (int eltsize, int size, char *name)
{

  int fd = -1;
  int map = MAP_SHARED;

  if ( strcmp(name,"") > 0 )
    {
      fd = shm_open(name,O_RDWR | O_CREAT,0666);

      if (fd == -1)
	{
	  perror("shm_open ");
	  return NULL;
	}
      if (ftruncate(fd,sizeof(struct channel)) == -1)
	{
	  perror("ftruncate ");
	  return NULL;
	}
    }
  else
    {
      map = map | MAP_ANONYMOUS;
    }
  
  channel_t c = mmap(NULL,sizeof(struct channel),PROT_READ|PROT_WRITE,map,fd,0);

  c->shared = CHANNEL_PROCESS_SHARED;

  if (c == MAP_FAILED)
    {
      perror("mmap(channel) ");
      return NULL;
    }

  c->fp = static_new_fifo (size, eltsize, CHANNEL_PROCESS_SHARED, fd);
  
  c->read = mmap(NULL,sizeof(sem_t),PROT_READ|PROT_WRITE,map,fd,0);

  if ( c->read == MAP_FAILED)
    {
      perror("mmap(semaphore read) ");
      return NULL;
    }
  
  c->write = mmap(NULL,sizeof(sem_t),PROT_READ|PROT_WRITE,map,fd,0);

  if ( c->write == MAP_FAILED)
    {
      perror("mmap(semaphore read) ");
      return NULL;
    }

  c->access = mmap(NULL,sizeof(sem_t),PROT_READ|PROT_WRITE,map,fd,0);

  if ( c->access == MAP_FAILED)
    {
      perror("mmap(semaphore read) ");
      return NULL;
    }
  
  c->eltsize = eltsize;
  c->size = size;
  c->open = CHANNEL_OPEN;
  if(c->fp == NULL){
    perror("static_new_fifo ");
    return NULL;
  }
  if(sem_init(c->read,1,0) == -1){
    perror ("sem_init(read)");
    return NULL;
  }
  
  if(sem_init(c->write,1,size) == -1){
    perror("sem_init(write)");
    return NULL;
  }

  if(sem_init(c->access,1,2) == -1){
    perror("sem_init(access)");
    return NULL;
  }
  
  if (c->fp == NULL)
    {
      perror ("new_fifo()");
      return NULL;
    }
  
  return c;
}
  
/** This subroutine allocates and initializes a channel (local or global according to the value of 'flags'),
 * then new channel is open. If successful a new channel is returned.
 * If not a NULL pointer is returned and errno set ENOMEM. */

/** TODO test errors after calling routines and
 * implement missing features */

struct channel *
channel_create (int eltsize, int size, int flags)
{
  channel_t c = NULL;
  if ( flags == CHANNEL_PROCESS_SHARED )
    {
      c = global_channel_create(eltsize,size,"");
    }
  else
    {
      c = local_channel_create(eltsize,size);
    }
  return c;
}

struct channel *
named_channel_create (int eltsize, int size, char *name)
{
  return global_channel_create(eltsize,size,name);
}

/** This subroutine free all dynamically allocated data and set all
 * channel pointers to NULL */

/** TODO test errors after calling routines and
 * implement missing features */

void 
channel_destroy (struct channel *channel)
{
  channel_t c = channel;
  sem_wait(c->access);

  sem_destroy(c->read);
  sem_destroy(c->write);
  sem_destroy(c->access);

  static_free_fifo (c->fp,c->shared);
  c->fp = NULL;
  if (c->shared == CHANNEL_PROCESS_SHARED) {
    munmap(c,sizeof(struct channel));
  } else {
    free (c);
  }
  c = NULL;
}

/** This subroutine closes an open channel.*/

/** TODO test errors after calling routines and
 * implement missing features */

int 
channel_close (struct channel *channel)
{
	channel_t c = channel;
	sem_wait(c->access);

	if (channel->open == CHANNEL_OPEN)
	{
		channel->open = CHANNEL_CLOSED;
		sem_post (c->access);
		return 1;
	}

	else if (channel->open == CHANNEL_CLOSED)
	{
		sem_post (c->access);
		return 0;
	}

	sem_post (c->access);
	return -1;
}

/** This subroutine sends data pointed by const void *data
 * to the channel. */

/** TODO test errors after calling routines and
 * implement missing features */

int 
channel_send (struct channel *channel, const void *data)
{
	channel_t c = channel;
	static_fifo_t *f = c->fp;
	if (c->size > 0) {
	  sem_wait(c->write);
	  sem_wait(c->access);

	  if (c->open == CHANNEL_CLOSED)
	    {
	      errno = EPIPE;
	      sem_post (c->write);
	      sem_post (c->access);
	      return -1;
	    }

	  int rc = static_fifo_add (f, (void *) data);
		
	  sem_post(c->read);
	  if ( rc == -1)
	    {
	      errno = rc;
	      perror ("channel_send(data)");
	      sem_post (c->write);
	      sem_post (c->access);
	      return -1;
	    }

	  sem_post (c->access);
	} else {
	  sem_wait(c->write);
	  memcpy(c->data,data,c->eltsize);
	  sem_post(c->read);
	}
	return 1;
}

/** This subroutine receives data in the channel in blocking mode
 * and set the pointer void *data to point to it */


/** TODO test errors after calling routines and
 * implement missing features */
int channel_recv (struct channel *channel, void *data)
{
  channel_t c = channel;
  static_fifo_t *f = c->fp;
  if (c->size > 0) {
    sem_wait(c->access);
    
    if ((static_fifo_len (f) == 0) && c->open == CHANNEL_CLOSED)
      {
	sem_post(c->access);
	return 0;
      }
    else{
      sem_post(c->access);
    }
    sem_wait(c->read);
    sem_wait(c->access);
    static_fifo_top (f, data);
    sem_post(c->write);
    sem_post(c->access);
  } else {
    sem_wait(c->access);
    c->data = data;
    sem_post(c->write);
    sem_wait(c->read);
    sem_post(c->access);
  }
  return 1;
}
