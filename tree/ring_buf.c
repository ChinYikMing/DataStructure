#include    <stdio.h>
#include    <stdlib.h>
#include    "ring_buf.h"
#include    "bst.h"

int rbuf_init(Rbuf *rbuf){
   rbuf->buf = malloc(sizeof(BST *) * RBUF_DFT_CAPACITY);
   if(!rbuf->buf) return ERR_MALLOC_FAILED;
   rbuf->capacity = RBUF_DFT_CAPACITY;
   rbuf->size = 0;
   rbuf->head = 0;
   rbuf->tail = 0;
   return 0;
}

int rbuf_destruct(Rbuf *rbuf){ 
    free(rbuf->buf);
    return 0;
}

int rbuf_push(Rbuf *rbuf, BST *node){
    if(rbuf_isfull(rbuf)){
	size_t new_capacity = rbuf->capacity << 1u;
	size_t data_cnt_after_head = rbuf->capacity - rbuf->head;
	void *tmp = realloc(rbuf->buf, sizeof(BST *) * new_capacity);
	if(!tmp) return ERR_MALLOC_FAILED;
	// invariant: tmp is malloc successfully
	rbuf->buf = tmp;

	/* reset the head if necessary */
	if(rbuf->head > rbuf->tail){
	    for(size_t i = rbuf->head, counter = data_cnt_after_head; i < rbuf->capacity; ++i, --counter)
		rbuf->buf[(i << 1u) + counter] = rbuf->buf[i]; 
	    
	    rbuf->head = (rbuf->head << 1u) + data_cnt_after_head;
	}

	rbuf->capacity = new_capacity;
    }
    // invariant: the rbuf has enough capacity to store data
    rbuf->buf[rbuf->tail] = malloc(sizeof(BST));
    rbuf->buf[rbuf->tail] = node;
    rbuf->tail = (rbuf->tail + 1) % rbuf->capacity;    
    rbuf->size++;

    return 0;
}

int rbuf_pop(Rbuf *rbuf){
    if(rbuf_isempty(rbuf)){
      printf("The ring buffer is empty!\n");
      return EXIT_FAILURE;
    }
    // invariant: the rbuf has entities
    int pop_val = (rbuf->buf[rbuf->head])->data;
    rbuf->head = (rbuf->head + 1) % rbuf->capacity;
    rbuf->size--;
    printf("%d\n", pop_val);
    
    return 0;
}

int rbuf_isfull(Rbuf *rbuf){
    return ((rbuf->tail + 1) % rbuf->capacity == rbuf->head);
}

int rbuf_isempty(Rbuf *rbuf){
    return (rbuf->size == 0);
}
