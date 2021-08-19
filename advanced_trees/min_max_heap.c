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
    int is_left;                 /* -1 is root, 0 is left, 1 is right */
    struct node *left;
    struct node *right;
    struct node *parent;
    struct node *prev_last_node;
} Node;

#define is_min_level(node) (_Bool)((uintptr_t) (node) & (0x01))  /* 1 is min level, 0 is max level */
#define mark_min_level(node) ((uintptr_t) (node) | (0x01))      /* 1 is min level, 0 is max level */
#define unmark_min_level(node) ((uintptr_t) (node) & (~0x01))
#define get_mark_min_level_node(node) ((Node *) mark_min_level(node))
#define get_unmark_min_level_node(node) ((Node *) unmark_min_level(node))

typedef struct min_max_heap {
    Node *root;
    Node *last_node;
} MinMaxHeap;

enum {
    DELETE_MIN,
    DELETE_MAX,
    VERIFY_MIN,
    VERIFY_MAX
};

static int mmheap_init(MinMaxHeap *mmheap);
static int mmheap_destroy(MinMaxHeap *mmheap);
static Node *mmheap_min(MinMaxHeap *mmheap, Node *from, Node **new_min_node_parent);
static Node *mmheap_max(MinMaxHeap *mmheap, Node *from, Node **new_max_node_parent);
static int mmheap_insert(MinMaxHeap *mmheap, int val);
static int mmheap_delete_min(MinMaxHeap *mmheap);
static int mmheap_delete_max(MinMaxHeap *mmheap);
static int mmheap_delete(MinMaxHeap *mmheap, int delete_min);
static int mmheap_print(MinMaxHeap *mmheap);
static Node **mmheap_find_next_pos(MinMaxHeap *mmheap, Node **next_pos, int *is_left); /* return is the next_pos */

static Node *node_new(int val);
static int node_swap_val(Node *n1, Node *n2);
static int node_verify(Node *node_challenge, int verify_min);
static int node_verify_min(Node *node_challenge);
static int node_verify_max(Node *node_challenge);
static Node *node_max(Node *root);

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

int main(){
    MinMaxHeap mmheap;
    mmheap_init(&mmheap);

    mmheap_insert(&mmheap, 1);
    mmheap_insert(&mmheap, 40);
    mmheap_insert(&mmheap, 90);
    mmheap_insert(&mmheap, 8);
    mmheap_insert(&mmheap, 10);
    mmheap_insert(&mmheap, 60);
    mmheap_insert(&mmheap, 50);
    mmheap_insert(&mmheap, 20);
    mmheap_insert(&mmheap, 30);
    mmheap_insert(&mmheap, 25);
    mmheap_insert(&mmheap, 18);
    mmheap_insert(&mmheap, 80);
    mmheap_insert(&mmheap, 70);

    mmheap_print(&mmheap);

    // int min = mmheap_delete_min(&mmheap);
    // printf("delete min: %d\n", min);
    // mmheap_print(&mmheap);
    // min = mmheap_delete_min(&mmheap);
    // printf("second delete min: %d\n", min);
    // mmheap_print(&mmheap);

    int max = mmheap_delete_max(&mmheap);
    printf("delete max: %d\n", max);
    mmheap_print(&mmheap);
    max = mmheap_delete_max(&mmheap);
    printf("second delete max: %d\n", max);
    mmheap_print(&mmheap);

    mmheap_destroy(&mmheap);
    return 0;
}

static int mmheap_init(MinMaxHeap *mmheap){
    mmheap->root = NULL;
}

static int mmheap_destroy(MinMaxHeap *mmheap){
    Node *node = mmheap->root;
    if(!node)
        return 0;

    mmheap->root = node->left;
    mmheap_destroy(mmheap);
    mmheap->root = node->right;
    mmheap_destroy(mmheap);
    free(node);
}

static int mmheap_insert(MinMaxHeap *mmheap, int val){
    Node **root = &mmheap->root;
    Node **next_pos, *next_pos_parent;
    Node *new_node = node_new(val);
    int is_left;

    if(!(*root)){
        *root = new_node;
        mmheap->last_node = *root; /* for delete min */
        return 0;
    }

    new_node->prev_last_node = mmheap->last_node; /* link new node with previous last node */
    mmheap->last_node = new_node; /* for delete min */

    next_pos = mmheap_find_next_pos(mmheap, &next_pos_parent, &is_left);
    *next_pos = new_node; /* place the new node in next_pos */
    (*next_pos)->parent = get_unmark_min_level_node(next_pos_parent);
    new_node->is_left = !(is_left == 0);

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

static Node **mmheap_find_next_pos(MinMaxHeap *mmheap, Node **next_pos_parent, int *is_left){
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
            *is_left = 0;
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
            *is_left = 1;
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

static int mmheap_print(MinMaxHeap *mmheap){
    Node *node = mmheap->root;
    if(!node){
        printf("empty\n");
        return 0;
    }

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

typedef Node *(*mmheap_delete_helper)(MinMaxHeap *, Node *, Node **); /* mmheap_max or mmheap_min */
static int mmheap_delete(MinMaxHeap *mmheap, int delete_min){
    if(delete_min != DELETE_MIN && delete_min != DELETE_MAX)
        return 0;

    Node *root = mmheap->root;
    Node *last_node = mmheap->last_node;
    Node *prev_last_node = last_node->prev_last_node;
    Node *new_max_node, *new_max_node_parent;
    Node *node = node_max(root);
    Node *new_node, *new_node_parent;
    mmheap_delete_helper helper;
    int ret;

    if(delete_min == DELETE_MAX){
        node = node_max(root);
        helper = mmheap_max;
    } else {
        node = root;
        helper = mmheap_min;
    }
    ret = node->val;

    node_swap_val(node, last_node);
    if(last_node->is_left == 0)
        last_node->parent->left = NULL;
    else if(last_node->is_left == 1)
        last_node->parent->right = NULL;
    free(last_node);
    mmheap->last_node = prev_last_node;

again:
    new_node = new_node_parent = NULL;

    if(!node->left && !node->right){
        if(!mmheap->last_node)       /* min max heap is empty */
            mmheap->root = NULL;
        goto end;
    } else {
        new_node = helper(mmheap, node, &new_node_parent);
        if(new_node){
            node_swap_val(new_node, node);
            if((node->left && node->left == new_node) || 
                    (node->right && node->right == new_node))
                    goto end;
            else {
                if(delete_min == DELETE_MIN){
                    if(new_node->val > new_node_parent->val){      
                        node_swap_val(new_node, new_node_parent);
                        node = new_node;
                        goto again;
                    }
                } else {
                    if(new_node->val < new_node_parent->val){
                        node_swap_val(new_node, new_node_parent);
                        node = new_node;
                        goto again;
                    }
                }
                goto end;
            }
        }
        goto end;
    }

end:
    return ret;
}

static int mmheap_delete_max(MinMaxHeap *mmheap){
    Node *root = mmheap->root;
    if(!root)
        return 0;

    return  mmheap_delete(mmheap, DELETE_MAX);
}

static int mmheap_delete_min(MinMaxHeap *mmheap){
    Node *root = mmheap->root;
    if(!root)
        return 0;

    return mmheap_delete(mmheap, DELETE_MIN);
}

/* todo: can be more effiency by skipping min level */
static Node *mmheap_max(MinMaxHeap *mmheap, Node *from, Node **new_max_node_parent){
    Node *node = from;
    int max = node->val;
    Node *new_max_node = NULL;
    Rbuf rbuf;
    rbuf_init(&rbuf);

    rbuf_push(&rbuf, node);
    while(!rbuf_isempty(&rbuf)){
        rbuf_pop(&rbuf);

        if(node->left){
            if(node->left->val > max){
                max = node->left->val;
                new_max_node = node->left;
                *new_max_node_parent = new_max_node->parent;
            }
            rbuf_push(&rbuf, node->left);
        }

        if(node->right){
            if(node->right->val > max){
                max = node->right->val;
                new_max_node = node->right;
                *new_max_node_parent = new_max_node->parent;
            }
            rbuf_push(&rbuf, node->right);
        }

        node = rbuf.buf[rbuf.head];
    }
    rbuf_destruct(&rbuf);

    return new_max_node; 
}

/* todo: can be more effiency by skipping max level */
static Node *mmheap_min(MinMaxHeap *mmheap, Node *from, Node **new_min_node_parent){
    Node *node = from;
    int min = node->val;
    Node *new_min_node = NULL;
    Rbuf rbuf;
    rbuf_init(&rbuf);

    rbuf_push(&rbuf, node);
    while(!rbuf_isempty(&rbuf)){
        rbuf_pop(&rbuf);

        if(node->left){
            if(node->left->val < min){
                min = node->left->val;
                new_min_node = node->left;
                *new_min_node_parent = new_min_node->parent;
            }
            rbuf_push(&rbuf, node->left);
        }

        if(node->right){
            if(node->right->val < min){
                min = node->right->val;
                new_min_node = node->right;
                *new_min_node_parent = new_min_node->parent;
            }
            rbuf_push(&rbuf, node->right);
        }

        node = rbuf.buf[rbuf.head];
    }
    rbuf_destruct(&rbuf);

    return new_min_node; 
}

int rbuf_init(Rbuf *rbuf){
   rbuf->buf = malloc(sizeof(Node *) * RBUF_DFT_CAPACITY);
   if(!rbuf->buf)
        err_exit("rbuf_init");
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
	if(!tmp)
        err_exit("realloc ring buffer in rbuf_push");
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
    if(rbuf_isempty(rbuf))
        err_exit("The ring buffer is empty");

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

static Node *node_new(int val){
    Node *new_node = malloc(sizeof(Node));
    if(!new_node)
        err_exit("malloc new node");
    new_node->val = val;
    new_node->is_left = -1;
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->parent = NULL;
    new_node->prev_last_node = NULL;

    return new_node;
}

static int node_swap_val(Node *n1, Node *n2){       /* do not check n1 or n2 is NULL, FIXME */
    int val1 = n1->val;
    n1->val = n2->val;
    n2->val = val1;
    return 0;
}

static Node *node_max(Node *root){
    Node *left = root->left;
    Node *right = root->right;
    
    if(left && right)
        return left->val > right->val ? left : right;
    else if(!left && right)
        return right;
    else if(left && !right)
        return left;

    return root;    /* root is the max */
}

typedef int(*node_verify_helper)(Node *); /* node_verify_min or node_verify_max */
static int node_verify(Node *node_challenge, int verify_min){
    node_verify_helper helper;

    if(verify_min == VERIFY_MIN)
        helper = node_verify_min;
    else
        helper = node_verify_max;

    if(!node_challenge)
        goto end;

    if(!node_challenge->parent)
        goto end;

    Node *node_challenge_target = node_challenge->parent->parent;
    if(!node_challenge_target)
        goto end;
    
    if(verify_min == VERIFY_MIN){
        if(node_challenge->val < node_challenge_target->val){
            node_swap_val(node_challenge, node_challenge_target);
            helper(node_challenge_target);
        }
    } else {
        if(node_challenge->val > node_challenge_target->val){
            node_swap_val(node_challenge, node_challenge_target);
            helper(node_challenge_target);
        }
    }

end:
    return 0;
}

static int node_verify_min(Node *node_challenge){
    return node_verify(node_challenge, VERIFY_MIN);
}

static int node_verify_max(Node *node_challenge){
    return node_verify(node_challenge, VERIFY_MAX);
}
