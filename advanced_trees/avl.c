/* 參考 https://zh.wikipedia.org/wiki/AVL%E6%A0%91 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

#define err_exit(msg) \
    do { \
        perror(msg); \
        exit(EXIT_FAILURE); \
    } while(0) \

#define max(x, y) ({ \
    __auto_type _x = (x); \
    __auto_type _y = (y); \
    _x > _y ? _x : _y; \
})

typedef struct node {
    int val;
    struct node *left;
    struct node *right;
    struct node *parent;
} Node;

#define get_parent(n) (Node *)((n->parent))

enum {
    LL,
    RR,
    LR,
    RL
};

typedef struct avl {
    Node *root;
    size_t size;
} AVL;

static int avl_init(AVL *avl);
static int avl_destroy(AVL *avl);
static int avl_insert(AVL *avl, int val);
static Node *avl_search(AVL *avl, int val);
static int avl_ll_rot(AVL *avl, Node *target_node);
static int avl_rr_rot(AVL *avl, Node *target_node);
static int avl_lr_rot(AVL *avl, Node *target_node);
static int avl_rl_rot(AVL *avl, Node *target_node);
static _Bool avl_is_balance(AVL *avl, Node **new_node, int *case_type);
static int avl_level_order_print(AVL *avl);
static int avl_in_order_print(AVL *avl);

static Node *node_new(int val);
static int tree_height(Node *start_node);

#define RBUF_DFT_CAPACITY 8u
typedef struct rbuf {
  Node **buf;
  size_t capacity;
  size_t size;
  size_t head;
  size_t tail;
} Rbuf;

static int rbuf_init(Rbuf *rbuf);
static int rbuf_destruct(Rbuf *rbuf);
static int rbuf_push(Rbuf *rbuf, Node *node);
static int rbuf_pop(Rbuf *rbuf);
static int rbuf_isfull(Rbuf *rbuf);
static int rbuf_isempty(Rbuf *rbuf);

int main(){
    AVL avl;
    avl_init(&avl);

    avl_insert(&avl, 2);
    avl_insert(&avl, 5);
    avl_insert(&avl, 8);
    avl_insert(&avl, 4);
    avl_insert(&avl, 3);
    avl_insert(&avl, 1);
    avl_insert(&avl, 9);
    avl_insert(&avl, 10);
    avl_insert(&avl, 7);
    avl_insert(&avl, 6);

    avl_level_order_print(&avl);

    if(avl_search(&avl, 6)){
        printf("found\n");
    } else {
        printf("Not found\n");
    }

    avl_destroy(&avl);
}

static int rbuf_init(Rbuf *rbuf){
   rbuf->buf = malloc(sizeof(Node *) * RBUF_DFT_CAPACITY);
   if(!rbuf->buf)
        err_exit("rbuf_init");
   rbuf->capacity = RBUF_DFT_CAPACITY;
   rbuf->size = 0;
   rbuf->head = 0;
   rbuf->tail = 0;
   return 0;
}

static int rbuf_destruct(Rbuf *rbuf){ 
    free(rbuf->buf);
    return 0;
}

static int rbuf_push(Rbuf *rbuf, Node *node){
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

static int rbuf_pop(Rbuf *rbuf){
    if(rbuf_isempty(rbuf))
        err_exit("The ring buffer is empty");

    // invariant: the rbuf has entities
    int pop_val = (rbuf->buf[rbuf->head])->val;
    rbuf->head = (rbuf->head + 1) % rbuf->capacity;
    rbuf->size--;
    
    return pop_val;
}

static int rbuf_isfull(Rbuf *rbuf){
    return ((rbuf->tail + 1) % rbuf->capacity == rbuf->head);
}

static int rbuf_isempty(Rbuf *rbuf){
    return (rbuf->size == 0);
}

static Node *node_new(int val){
    Node *new_node = malloc(sizeof(Node));
    if(!new_node)
        err_exit("malloc new node");
    new_node->val = val;
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->parent = NULL;

    return new_node;
}

static int avl_init(AVL *avl){
    avl->root = NULL;
    avl->size = 0;
}

static int avl_destroy(AVL *avl){
    Node *node = avl->root;
    if(!node)
        return 0;

    avl->root = node->left;
    avl_destroy(avl);
    avl->root = node->right;
    avl_destroy(avl);
    free(node);
}

static int avl_insert(AVL *avl, int val){
    Node *parent = NULL;
    Node **ptr = &avl->root;
    Node *node;
    Node *new_node;
    
    new_node = node_new(val);
    while((node = *ptr)){
        parent = node;
        if(new_node->val > node->val)
            ptr = &node->right;
        else if(new_node->val < node->val)
            ptr = &node->left;
    }
    new_node->parent = parent;
    *ptr = new_node;
    avl->size++;

    int case_type;
    if(!avl_is_balance(avl, &new_node, &case_type)){
        switch(case_type){
            case LL:
                printf("LL\n");
                avl_ll_rot(avl, new_node->parent);
                break;

            case RR:
                printf("RR\n");
                avl_rr_rot(avl, new_node->parent);
                break;

            case LR:
                printf("LR\n");
                avl_lr_rot(avl, new_node);
                break;

            case RL:
                printf("RL\n");
                avl_rl_rot(avl, new_node);
                break;

            default: /* should not reach here */
                break;
        }
    }

    return 0;
}

static int avl_level_order_print(AVL *avl){
    Node *node = avl->root;
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

static _Bool avl_is_balance(AVL *avl, Node **new_node, int *case_type){
    Node *parent = get_parent((*new_node));
    if(!parent)
        goto bal;

    Node *grandparent = get_parent(parent);
    if(!grandparent)
        goto bal;

    int left_par;
    int right_par;
    int left_grpar; 
    int right_grpar;
    int height_left;
    int height_right;

    /* start from grandparent of new_node until reach root */
    do {
        height_left = tree_height(grandparent->left);
        height_right = tree_height(grandparent->right);
        left_grpar = -1;
        right_grpar = -1;
        left_par = -1;
        right_par = -1;

        if(abs(height_left - height_right) > 1){
            if((*new_node)->val < parent->val){
                left_par = 1;
                right_par = 0;
            } else {
                left_par = 0;
                right_par = 1;
            }

            if((*new_node)->val < grandparent->val){
                left_grpar = 1;
                right_grpar = 0;
            } else {
                left_grpar = 0;
                right_grpar = 1;
            }

            if(left_grpar == 1 && left_par == 1)
                *case_type = LL;
            else if(right_grpar == 1 && right_par == 1)
                *case_type = RR;
            else if(left_grpar == 1 && right_par == 1)
                *case_type = LR;
            else
                *case_type = RL;

            return false;
        }

        *new_node = parent;
        parent = grandparent;
        grandparent = get_parent(grandparent);
    } while(grandparent);

bal:
    return true;
}

static int tree_height(Node *start_node){
    Node *node = start_node;

    if(!node)
        return 0;

    return max(1 + tree_height(node->left), 1 + tree_height(node->right));
}

static int avl_ll_rot(AVL *avl, Node *target_node){
    Node *parent = get_parent(target_node);
    Node *grandparent = get_parent(parent);

    /* right rotation */
    parent->left = target_node->right;
    if(target_node->right)
        target_node->right->parent = parent;
    target_node->right = parent;
    parent->parent = target_node;

    if(parent == avl->root){ /* if parent is root, then need to change to target_node */
        avl->root = target_node;
        target_node->parent = NULL;
    } else { /* need to change grandparent->right(left) to target_node */
        if(grandparent->val > parent->val)
            grandparent->left = target_node;
        else
            grandparent->right = target_node;
        target_node->parent = grandparent;
    }

    return 0;
}

static int avl_rr_rot(AVL *avl, Node *target_node){
    Node *parent = get_parent(target_node);
    Node *grandparent = get_parent(parent);

    /* left rotation */
    parent->right = target_node->left;
    if(target_node->left)
        target_node->left->parent = parent;
    target_node->left = parent;
    parent->parent = target_node;

    if(parent == avl->root){ /* if parent is root, then need to change to target_node */
        avl->root = target_node;
        target_node->parent = NULL;
    } else { /* need to change grandparent->right(left) to target_node */
        if(grandparent->val > parent->val)
            grandparent->left = target_node;
        else
            grandparent->right = target_node;
        target_node->parent = grandparent;
    }
    
    return 0;
}

static int avl_lr_rot(AVL *avl, Node *target_node){
    /* left rotation */
    Node *parent = get_parent(target_node);
    Node *grandparent = get_parent(parent);

    grandparent->left = target_node;
    target_node->parent = grandparent;
    parent->right = target_node->left;
    if(target_node->left)
        target_node->right->parent = parent;
    target_node->left = parent;
    parent->parent = target_node;

    /* note: rotation in avl_ll_rot is right rotation */
    avl_ll_rot(avl, target_node);

    return 0;
}

static int avl_rl_rot(AVL *avl, Node *target_node){
    /* right rotation */
    Node *parent = get_parent(target_node);
    Node *grandparent = get_parent(parent);

    grandparent->right = target_node;
    target_node->parent = grandparent;
    parent->left = target_node->right;
    if(target_node->right)
        target_node->right->parent = parent;
    target_node->right = parent;
    parent->parent = target_node;

    /* rotation in val_rr_rot is left rotation */
    avl_rr_rot(avl, target_node);

    return 0;
}

static Node *avl_search(AVL *avl, int val){
    if(avl->size == 0) /* empty */
        goto end;
    
    Node **ptr = &avl->root;
    Node *node;

    while((node = *ptr)){
        if(val == node->val) /* found */
            return node;
        else if(val > node->val)
            ptr = &node->right;
        else if(val < node->val)
            ptr = &node->left;
    }

end:
    return NULL;
}