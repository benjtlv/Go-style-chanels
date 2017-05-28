#ifndef _FIFO_H_
#define _FIFO_H_

typedef struct node_s
{
	void *n_data;
	struct node_s *n_next;
}
node_t;

typedef struct fifo_s
{
	node_t *f_top;
	node_t *f_tip;
}
fifo_t;

fifo_t * new_fifo (void);

int fifo_add (fifo_t *, void *);

void *fifo_top (fifo_t *, void *);

void *fifo_remove_node (fifo_t *, node_t *, void *);

void free_fifo (fifo_t *, void (*) (void *));

int fifo_len (fifo_t *);

int fifo_empty (fifo_t *);

#endif /* _FIFO_H_ */
