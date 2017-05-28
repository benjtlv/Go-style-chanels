#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "fifo.h"

fifo_t *
new_fifo (void)
{
	fifo_t *f;
	f = (fifo_t *) malloc (sizeof (fifo_t));

	if (f == NULL && errno == ENOMEM)
	{
		perror ("malloc(new_fifo)");
		return NULL;
	}

	return f;
}

int
fifo_add(fifo_t *f, void *data)
{
	node_t *fn;
	fn = (node_t *) malloc (sizeof (node_t));

	if (fn == NULL && errno == ENOMEM)
	{
		perror("malloc(node)");
		return -1;
	}
	
	fn->n_data = data;
	fn->n_next = NULL;
	
	if (f->f_tip == NULL)
	{

	
		f->f_top = f->f_tip = fn;
	}
	else
	{
			f->f_tip->n_next = fn;
			f->f_tip = fn;
	}
	return 0;
}

void *
fifo_top(fifo_t *f, void *data)
{
	
	node_t *fn;

	if ((fn = f->f_top) == NULL)
	{
		data = NULL;
	}
	else
	{
		data = fn->n_data;
	}
	
	if ((fn->n_next) == NULL)
	{
		f->f_tip = NULL;
	}

	f->f_top = fn->n_next;
	fn->n_next = NULL;
	free(fn);
	fn = NULL;

	return data;
}


void
free_fifo(fifo_t *f, void (*freedata)(void *))
{
	node_t *fn = f->f_top;
	node_t *tmp;

	if (freedata == NULL)
	{
		fprintf(stderr, "free_fifo: no free data subroutine.\n");
		exit(-1);
	}
	
	while (fn)
	{
		(*freedata)(fn->n_data);

		tmp = fn;
		fn = fn->n_next;
		free(tmp);
	}

	free(f);
}


int
fifo_len(fifo_t *f)
{
	node_t *fn;
	int i;

	for (i = 0, fn = f->f_top; fn; fn = fn->n_next, i++);

	return (i);
}

int
fifo_empty(fifo_t *f)
{
	return (f->f_top == NULL);
}
