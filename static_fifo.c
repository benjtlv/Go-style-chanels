#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
//#include <sys/types.h>
//#include <unistd.h>
#include <errno.h>
#include "static_fifo.h"

#define CHANNEL_PROCESS_SHARED 1

static_fifo_t *
static_new_fifo_local (int se)
{
  static_fifo_t *f = (static_fifo_t *) malloc ( sizeof(static_fifo_t) );

  if ( f == NULL && errno == ENOMEM)
    {
      perror("malloc(fifo) ");
      return NULL;
    }
  
  memset (f, 0, sizeof (static_fifo_t));

  f->buffer = malloc (se);
  
  if ( f->buffer == NULL && errno ==ENOMEM )
    {
      perror("malloc(fifo buffer) ");
      return NULL;
    }
  
  return f;
}

static_fifo_t *
static_new_fifo_global (int se, int fd)
{
  int map = MAP_SHARED;

  if (fd == -1)
    {
      map = map | MAP_ANONYMOUS;
    }
  
  static_fifo_t *f = mmap(NULL,sizeof(static_fifo_t),PROT_READ|PROT_WRITE,map,fd,0);

  if (f == MAP_FAILED)
    {
      perror("mmap(fifo) ");
      return NULL;
    }
  
  memset (f, 0, sizeof (static_fifo_t));

  f->buffer = mmap(NULL,se,PROT_READ|PROT_WRITE,map,fd,0);

  if (f->buffer == MAP_FAILED)
    {
      perror("mmap(fifo buffer) ");
      return NULL;
    }

  return f;
}

static_fifo_t *
static_new_fifo (int size, int eltsize, int flag, int fd)
{
	size_t se         = size * eltsize;
	static_fifo_t *f = NULL;
	if ( flag == CHANNEL_PROCESS_SHARED)
	  {
	    f = static_new_fifo_global(se,fd);
	  }
	else
	  {
	    f = static_new_fifo_local(se);
	  }
	if ( f != NULL)
	  {
            memset (f->buffer, 0, se);
	    f->eltsize        = eltsize;
	    f->size           = size;
	    f->top            = 0;
	    f->tail           = 0;
	    f->count          = 0;
	    return f;
          }
	else
	  {
            return NULL;
          }
}

int
static_fifo_add (static_fifo_t *f, void *data)
{
	if ( f->count == (f->size) )
	{
		return -1;
	}
	else
	{
		memcpy( ( f->buffer + (f->eltsize * f->tail) ), data, f->eltsize );
		f->tail = (f->tail + 1) % f->size;
		f->count++;
		return 0;
	}
}

void *
static_fifo_top (static_fifo_t *f, void *data)
{
	if ( f->count == 0 )
	{
		return NULL;
	}
	else
	{
		memcpy (data, (f->buffer + (f->top * f->eltsize)), f->eltsize);
		memset (f->buffer + (f->eltsize * f->top), 0, f->eltsize);
		f->count--;
		f->top = (f->top + 1) % f->size;
		return data;
	}
}

size_t
static_fifo_len (static_fifo_t *f)
{
	return f->count;
}

void
static_free_fifo (static_fifo_t *f, int shared)
{
  if (shared == CHANNEL_PROCESS_SHARED){
    munmap(f->buffer,sizeof(f->eltsize * f->size));
    munmap(f,sizeof(static_fifo_t));
  } else {
    free (f->buffer);
    free (f);
  }
  f->buffer = NULL;
  f = NULL;
}

void
static_fifo_print (static_fifo_t *f)
{
	int i;
	printf(" %s  Fifo%2s\n", "|", "|");
	for (i = 0; i < f->count; i++)
	{
		int pos = (int) ((f->top + i) % f->size);
		int val = *( (int *) (f->buffer + ((f->top + i) % f->size) * f->eltsize));
		printf("   %d : %d\n", pos, val);
	}
}

/* int */
/* main (int argc, char *argv[]) */
/* { */
/* 	int *p = malloc (sizeof (int)); */
/* 	int i; */
/* 	static_fifo_t *fifo = static_new_fifo (5,sizeof(int)); */
/*  */
/* 	for (i=2; i < 10; i = i + 2) */
/* 	{ */
/* 		static_fifo_add(fifo,&i); */
/* 		printf("Added %d\n",i); */
/* 	} */
/*  */
/* 	static_fifo_print(fifo); */
/* 	printf("%s\n", "  -------"); */
/*  */
/* 	*p = 10; */
/* 	static_fifo_add(fifo,p); */
/* 	static_fifo_print(fifo); */
/* 	printf("%s\n", "  -------"); */
/*  */
/* 	*p = 12; */
/* 	static_fifo_add(fifo,p); */
/* 	static_fifo_print(fifo); */
/* 	printf("%s\n", "  -------"); */
/* 	static_fifo_top (fifo, p);  */
/* 	printf("Enlevé : %d\n", *p); */
/*  */
/* 	*p = 12; */
/* 	static_fifo_add(fifo,p); */
/* 	static_fifo_print(fifo); */
/* 	printf("%10s\n", "  -------"); */
/*  */
/* 	static_free_fifo(fifo); */
/* 	exit  (EXIT_SUCCESS); */
/* } */
