#ifndef _STATIC_FIFO_H_
#define _STATIC_FIFO_H_

typedef struct static_fifo_s
{
	size_t eltsize;
	size_t size;
	size_t top;
	size_t tail;
	size_t count;
  	void *buffer;
}
static_fifo_t;

static_fifo_t *
static_new_fifo_local (int);

static_fifo_t *
static_new_fifo_global (int, int);

static_fifo_t *
static_new_fifo (int, int, int, int);

int
static_fifo_add (static_fifo_t *, void *);

void *
static_fifo_top (static_fifo_t *, void *);

size_t
static_fifo_len (static_fifo_t *);

void
static_free_fifo (static_fifo_t *,int);

#endif

void
static_fifo_print (static_fifo_t *);

