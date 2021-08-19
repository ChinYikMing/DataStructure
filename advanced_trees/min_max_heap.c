#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <stdalign.h>
#include <inttypes.h>

#define err_exit(msg) \
    do { \
        perror(msg); \
        exit(EXIT_FAILURE); \
    } while(0) \

typedef struct node {
    int val;
    struct node *left;
    struct node *right;
    struct node *parent;
} Node;

#define is_min_level(node) (_Bool)((uintptr_t) (node) & (0x01))  /* 1 is min level, 0 is max level */
#define mark_min_level(node) ((uintptr_t) (node) | (0x01))      /* 1 is min level, 0 is max level */
#define unmark_min_level(node) ((uintptr_t) (node) & (~0x01))
#define get_mark_min_level_node(node) ((Node *) mark_min_level(node))
#define get_unmark_min_level_node(node) ((Node *) unmark_min_level(node))

typedef struct min_max_heap {
    Node *root;
    int height;
} MinMaxHeap;

int mmheap_init(MinMaxHeap *mmheap);
int mmheap_destroy(MinMaxHeap *mmheap);
int mmheap_min(MinMaxHeap *mmheap);
int mmheap_max(MinMaxHeap *mmheap);
int mmheap_insert(MinMaxHeap *mmheap, int val);
int mmheap_delete_min(MinMaxHeap *mmheap);
int mmheap_delete_max(MinMaxHeap *mmheap);
int mmheap_print(MinMaxHeap *mmheap);
Node **mmheap_find_next_pos_and_next_pos_parent(MinMaxHeap *mmheap, Node **next_pos); /* return is the next_pos */

Node *node_new(int val);
int node_swap_val(Node *n1, Node *n2);
int node_verify_min(Node *node_challenge);
int node_verify_max(Node *node_challenge);

#define	    RBUF_DFT_CAPACITY	8u
#define	    ERR_MALLOC_FAILED	-1

typedef struct rbuf {
  Node **buf;
  size_t capacity;
  size_t size;
  size_t head;
  size_t tail;
} Rbuf;

int rbuf_init(Rbuf *rbuf);
int rbuf_destruct(Rbuf *rbuf);
int rbuf_push(Rbuf *rbuf, Node *node);
int rbuf_pop(Rbuf *rbuf);
int rbuf_isfull(Rbuf *rbuf);
int rbuf_isempty(Rbuf *rbuf);

void in(Node *n){
    if(!n)
        return;
    in(n->left);
    printf("%d\n", n->val);
    in(n->right);
}

int main(){
    MinMaxHeap mmheap;
    mmheap_init(&mmheap);

    // mmheap_insert(&mmheap, 10);
    // mmheap_insert(&mmheap, 30);
    // mmheap_insert(&mmheap, 40);
    // mmheap_insert(&mmheap, 25);
    // mmheap_insert(&mmheap, 22);
    // mmheap_insert(&mmheap, 15);
    // mmheap_insert(&mmheap, 8);
    // mmheap_insert(&mmheap, 50);
    // mmheap_insert(&mmheap, 3);
    // mmheap_insert(&mmheap, 90);

    mmheap_print(&mmheap);
    // printf("in order\n");
    // in(mmheap.root); 

    // mmheap_delete_min(&mmheap);

    // mmheap_print(&mmheap);

    mmheap_destroy(&mmheap);
    return 0;
}

int mmheap_init(MinMaxHeap *mmheap){
    mmheap->root = NULL;
    mmheap->height = 0;
}

int mmheap_destroy(MinMaxHeap *mmheap){
    Node *node = mmheap->root;
    if(!node)
        return 0;

    mmheap->root = node->left;
    mmheap_destroy(mmheap);
    mmheap->root = node->right;
    mmheap_destroy(mmheap);
    free(node);
}

int mmheap_min(MinMaxHeap *mmheap);

int mmheap_max(MinMaxHeap *mmheap);

int mmheap_insert(MinMaxHeap *mmheap, int val){
    Node **root = &mmheap->root;
    Node **next_pos, *next_pos_parent;
    Node *new_node = node_new(val);
    
    if(!(*root)){
        *root = new_node;
        return 0;
    }

    next_pos = mmheap_find_next_pos_and_next_pos_parent(mmheap, &next_pos_parent);
    *next_pos = new_node; /* place the new node in next_pos */
    (*next_pos)->parent = get_unmark_min_level_node(next_pos_parent);

    if(is_min_level(next_pos_parent)){
        next_pos_parent = get_unmark_min_level_node(next_pos_parent);
        if(val < next_pos_parent->val){
            node_swap_val(*next_pos, next_pos_parent);
            node_verify_min(next_pos_parent); /* challenge grantparent with next_pos_parent */
        } else {
            node_verify_max(*next_pos); /* challenge grantparent from next_pos */
        }
    } else {
        if(val > next_pos_parent->val){
            node_swap_val(*next_pos, next_pos_parent);
            node_verify_max(next_pos_parent); /* challenge grantparent from next_pos_parent */
        } else {
            node_verify_min(*next_pos); /* challenge grantparent from next_pos */
        }
    }

    return 0;
}

int node_verify_min(Node *node_challenge){
    if(!node_challenge)
        goto end;

    if(!node_challenge->parent)
        goto end;

    Node *node_challenge_target = node_challenge->parent->parent;
    if(!node_challenge_target)
        goto end;
    
    if(node_challenge->val < node_challenge_target->val){
        node_swap_val(node_challenge, node_challenge_target);
        node_verify_min(node_challenge_target);
    }

end:
    return 0;
}

int node_verify_max(Node *node_challenge){
    if(!node_challenge)
        goto end;

    if(!node_challenge->parent)
        goto end;

    Node *node_challenge_target = node_challenge->parent->parent;
    if(!node_challenge_target)
        goto end;
    
    if(node_challenge->val > node_challenge_target->val){
        node_swap_val(node_challenge, node_challenge_target);
        node_verify_max(node_challenge_target);
    }

end:
    return 0;
}

Node *node_new(int val){
    Node *new_node = malloc(sizeof(Node));
    if(!new_node)
        err_exit("malloc new node");
    new_node->val = val;
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->parent = NULL;

    return new_node;
}

int node_swap_val(Node *n1, Node *n2){       /* do not check n1 or n2 is NULL, FIXME */
    int val1 = n1->val;
    n1->val = n2->val;
    n2->val = val1;
    return 0;
}

Node **mmheap_find_next_pos_and_next_pos_parent(MinMaxHeap *mmheap, Node **next_pos_parent){
    Node *node = mmheap->root;
    Rbuf rbuf;
    rbuf_init(&rbuf);

    int curr_level = 1;
    int max_node_pop_cnt_curr_level = (1 << curr_level) - 1;       /* n = 2^h - 1 */
    int node_pop_cnt = 0;

    rbuf_push(&rbuf, node);
    while(!rbuf_isempty(&rbuf)){
        *next_pos_parent = node;

        if(node_pop_cnt == max_node_pop_cnt_curr_level){
            curr_level++;
            max_node_pop_cnt_curr_level = (1 << curr_level) - 1;
        }

        rbuf_pop(&rbuf);
        node_pop_cnt++;

        if(!node->left){
            if((*next_pos_parent == mmheap->root) || (curr_level % 2 != 0)) /* node is in min level */
                *next_pos_parent = get_mark_min_level_node(node);
            else
                *next_pos_parent = node;
            rbuf_destruct(&rbuf);
            return &node->left;
        } else {
            rbuf_push(&rbuf, node->left);
        }

        if(!node->right){
            if((*next_pos_parent == mmheap->root) || (curr_level % 2 != 0)) /* node is in min level */
                *next_pos_parent = get_mark_min_level_node(node);
            else
                *next_pos_parent = node;
            rbuf_destruct(&rbuf);
            return &node->right;
        } else {
            rbuf_push(&rbuf, node->right);
        }

        node = rbuf.buf[rbuf.head];
    }
    rbuf_destruct(&rbuf);

    return NULL; /* should not reach here */
}

int mmheap_print(MinMaxHeap *mmheap){
    Node *node = mmheap->root;
    if(!node)
        return 0;

    Rbuf rbuf;
    int ret;
    rbuf_init(&rbuf);

    rbuf_push(&rbuf, node);
    while(!rbuf_isempty(&rbuf)){
        ret = rbuf_pop(&rbuf);
        printf("%d\n", ret);

        if(node->left)
            rbuf_push(&rbuf, node->left);

        if(node->right)
            rbuf_push(&rbuf, node->right);

        node = rbuf.buf[rbuf.head];
    }
    rbuf_destruct(&rbuf);

    return 0;
}

int rbuf_init(Rbuf *rbuf){
   rbuf->buf = malloc(sizeof(Node *) * RBUF_DFT_CAPACITY);
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

int rbuf_push(Rbuf *rbuf, Node *node){
    if(rbuf_isfull(rbuf)){
	size_t new_capacity = rbuf->capacity << 1u;
	size_t val_cnt_after_head = rbuf->capacity - rbuf->head;
	void *tmp = realloc(rbuf->buf, sizeof(Node *) * new_capacity);
	if(!tmp) return ERR_MALLOC_FAILED;
	// invariant: tmp is malloc successfully
	rbuf->buf = tmp;

	/* reset the head if necessary */
	if(rbuf->head > rbuf->tail){
	    for(size_t i = rbuf->head, counter = val_cnt_after_head; i < rbuf->capacity; ++i, --counter)
		rbuf->buf[(i << 1u) + counter] = rbuf->buf[i]; 
	    
	    rbuf->head = (rbuf->head << 1u) + val_cnt_after_head;
	}

	rbuf->capacity = new_capacity;
    }
    // invariant: the rbuf has enough capacity to store val
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
    int pop_val = (rbuf->buf[rbuf->head])->val;
    rbuf->head = (rbuf->head + 1) % rbuf->capacity;
    rbuf->size--;
    
    return pop_val;
}

int rbuf_isfull(Rbuf *rbuf){
    return ((rbuf->tail + 1) % rbuf->capacity == rbuf->head);
}

int rbuf_isempty(Rbuf *rbuf){
    return (rbuf->size == 0);
}

int mmheap_delete_min(MinMaxHeap *mmheap){

}

int mmheap_delete_max(MinMaxHeap *mmheap){

}
