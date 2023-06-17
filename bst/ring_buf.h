#ifndef	    RING_BUF_H
#define	    RING_BUF_H

#include    <stddef.h>
#include    "bst.h"
#define	    RBUF_DFT_CAPACITY	8u
#define	    ERR_MALLOC_FAILED	-1

typedef struct rbuf {
  BST **buf;
  size_t capacity;
  size_t size;
  size_t head;
  size_t tail;
} Rbuf;

int rbuf_init(Rbuf *rbuf);

int rbuf_destruct(Rbuf *rbuf);

int rbuf_push(Rbuf *rbuf, BST *node);

int rbuf_pop(Rbuf *rbuf);

int rbuf_isfull(Rbuf *rbuf);

int rbuf_isempty(Rbuf *rbuf);

#endif
