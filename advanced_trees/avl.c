/* 參考 https://zh.wikipedia.org/wiki/AVL%E6%A0%91 */
/* 參考 https://iter01.com/437465.html */

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

enum rot_case_type {
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
/* a lot can be refactor, FIXME */
static int avl_delete(AVL *avl, int val);
static int avl_ll_rot(AVL *avl, Node *target_node);
static int avl_rr_rot(AVL *avl, Node *target_node);
static int avl_lr_rot(AVL *avl, Node *target_node);
static int avl_rl_rot(AVL *avl, Node *target_node);
static _Bool avl_is_balance(AVL *avl, Node **new_node, int *rot_case_type);
static int avl_balance(AVL *avl, Node *new_node, int case_type);
static int avl_level_order_print(AVL *avl);
static int avl_in_order_print(AVL *avl);

static Node *node_new(int val);
/**
 * to fulfil avl_is_balance implementation since 
 * avl_is_balance always needs parent and grandparent 
 * so, this function helps find the node which guarantees has parent and grandparent
 */
static Node *find_new_node_for_balance(AVL *avl, Node *node_to_delete);
static int tree_height(Node *start_node);
static Node *bfs(Node *start_node); // find next to balance target node

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
static Node *rbuf_pop(Rbuf *rbuf);
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

    avl_delete(&avl, 5);
    avl_delete(&avl, 4);
    avl_delete(&avl, 3);
    avl_delete(&avl, 8);
    avl_delete(&avl, 7);
    avl_delete(&avl, 6);
    avl_delete(&avl, 2);
    avl_delete(&avl, 9);
    avl_delete(&avl, 1);
    avl_delete(&avl, 10);
    
    avl_level_order_print(&avl);

    int to_search = 6;
    if(avl_search(&avl, to_search)){
        printf("%d is found\n", to_search);
    } else {
        printf("%d is not found\n", to_search);
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

static Node *rbuf_pop(Rbuf *rbuf){
    if(rbuf_isempty(rbuf))
        err_exit("The ring buffer is empty");

    // invariant: the rbuf has entities
    Node *ret = rbuf->buf[rbuf->head];
    rbuf->head = (rbuf->head + 1) % rbuf->capacity;
    rbuf->size--;
    
    return ret;
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

    int rot_case_type;
    if(!avl_is_balance(avl, &new_node, &rot_case_type))
        avl_balance(avl, new_node, rot_case_type);

    return 0;
}

static int avl_level_order_print(AVL *avl){
    Node *node = avl->root;
    if(!node){
        printf("empty\n");
        return 0;
    }

    Rbuf rbuf;
    Node *ret;
    rbuf_init(&rbuf);
    rbuf_push(&rbuf, node);
    while(!rbuf_isempty(&rbuf)){
        ret = rbuf_pop(&rbuf);
        printf("%d\n", ret->val);

        if(node->left)
            rbuf_push(&rbuf, node->left);

        if(node->right)
            rbuf_push(&rbuf, node->right);

        node = rbuf.buf[rbuf.head];
    }
    rbuf_destruct(&rbuf);

    return 0;
}

static _Bool avl_is_balance(AVL *avl, Node **new_node, int *rot_case_type){
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
                *rot_case_type = LL;
            else if(right_grpar == 1 && right_par == 1)
                *rot_case_type = RR;
            else if(left_grpar == 1 && right_par == 1)
                *rot_case_type = LR;
            else
                *rot_case_type = RL;

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
        target_node->left->parent = parent;
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

static int avl_delete(AVL *avl, int val){
    Node *node_to_delete = avl_search(avl, val);
    if(!node_to_delete)
        return 0;

    Node *parent = get_parent(node_to_delete);
    Node *new_node = NULL; // for balancing
    Node *left_largest = NULL; // for balancing

    avl->size--;
    /** three cases:
     *  (1) node_to_delete is leaf node
     *  (2) node_to_delete has one child(replace with child(left or right))
     *  (3) node_to_delete has two child(choose left_largest in left to replace or smallest in right to replace to remain in order)
     */
    if(!node_to_delete->left && !node_to_delete->right){ /* case (1) */
        if(parent){
            if(parent->val > node_to_delete->val) /* node_to_delete is left */
                parent->left = NULL;               
            else
                parent->right = NULL;               
            new_node = parent;
        } else { /* node_to_delete is root */
            avl->root = NULL;
        }
        left_largest = parent ? parent : NULL;
        printf("delete %d is case LEAF\n", val);
    } else if(!node_to_delete->right && node_to_delete->left){ /* case (2) has left child */
        if(parent){
            if(parent->val > node_to_delete->val) /* node_to_delete is left */
                parent->left = node_to_delete->left;               
            else
                parent->right = node_to_delete->left;               
            node_to_delete->left->parent = parent;
        } else { /* node_to_delete is root */
            avl->root = node_to_delete->left;
            node_to_delete->left->parent = NULL;
        }
        left_largest = node_to_delete->left;
        printf("delete %d is case ONE_CHILD_LEFT\n", val);
    } else if(!node_to_delete->left && node_to_delete->right){ /* case (2) has right child */
        if(parent){
            if(parent->val > node_to_delete->val) /* node_to_delete is left */
                parent->left = node_to_delete->right;               
            else
                parent->right = node_to_delete->right;               
            node_to_delete->right->parent = parent;
        } else { /* node_to_delete is root */
            avl->root = node_to_delete->right;
            node_to_delete->right->parent = NULL;
        }
        left_largest = node_to_delete->right;
        printf("delete %d is case ONE_CHILD_RIGHT\n", val);
    } else { /* case (3) */
        /* here choosing left left_largest child to replace */
        Node *left_largest_parent;
        Node *n = node_to_delete->left;
        while(n){
            left_largest = n;
            left_largest_parent = get_parent(left_largest);
            n = n->right;
        }

        if(left_largest != node_to_delete->left){ /* change left_largest->left only if left_largest is not node_to_delete->left */
            left_largest_parent->right = left_largest->left; /* linking left_largest->left */
            if(left_largest->left)
                left_largest->left->parent = left_largest_parent;
            left_largest->left = node_to_delete->left;
            if(node_to_delete->left)
                node_to_delete->left->parent = left_largest;
        }
        left_largest->right = node_to_delete->right;
        if(node_to_delete->right)
            node_to_delete->right->parent = left_largest;

        if(parent){
            if(parent->val > node_to_delete->val) /* node_to_delete is left */
                parent->left = left_largest;               
            else
                parent->right = left_largest;               
            left_largest->parent = parent;
        } else { /* node_to_delete is root */
            avl->root = left_largest;
            left_largest->parent = NULL;
        }
        printf("delete %d is case TWO_CHILD\n", val);
    }

    /* rebalance the tree if necessary, if new_node is not NULL then it is imbalance */
    new_node = find_new_node_for_balance(avl, left_largest);
    while(new_node){ /* could be many rotation to rebalance */
        int rot_case_type;
        if(!avl_is_balance(avl, &new_node, &rot_case_type))
            avl_balance(avl, new_node, rot_case_type);
        
        new_node = find_new_node_for_balance(avl, new_node);
    }

    free(node_to_delete);
    return 0;
}

static Node *find_new_node_for_balance(AVL *avl, Node *node_to_delete){
    Node *new_node = NULL;  // if new_node is NULL at the end, then it means balance
    Node *parent = node_to_delete ? get_parent(node_to_delete) : NULL;
    Node *node = node_to_delete;
    int left_height;
    int right_height;

    if(parent){ /* node_to_delete has parent, so check node_to_delele and parent(s) */
        do {
            left_height = tree_height(node_to_delete->left);
            right_height = tree_height(node_to_delete->right);

            if(right_height == (left_height - 2)){ /* left has more nodes */
                if(node_to_delete->left->left)
                    new_node = bfs(node_to_delete->left); // find deepest node
                break;
            } else if(left_height == (right_height - 2)){ /* right has more nodes */
                if(node_to_delete->right->right)
                    new_node = bfs(node_to_delete->right); // find deepest node
                break;
            }
            node_to_delete = get_parent(node_to_delete);
        } while(node_to_delete);
    } else { /* node_to_delete is root(no parent) and we always use largest_left to replace root, so check new root and left only */
        /* check root if any */
        if(avl->root){
            left_height = tree_height(avl->root->left);
            right_height = tree_height(avl->root->right);

            if(right_height == (left_height - 2)){ /* left has more nodes */
                if(avl->root->left)
                    new_node = bfs(avl->root->left);
            } else if(left_height == (right_height - 2)){ /* right has more nodes */
                if(avl->root->right)
                    new_node = bfs(avl->root->right);
            }

            if(!new_node){ /* if new_node is still NULL, check root left */
                if(avl->root->left){
                    left_height = tree_height(avl->root->left->left);
                    right_height = tree_height(avl->root->left->right);

                    if(right_height == (left_height - 2)){ /* left has more nodes */
                        if(avl->root->left->left)
                            new_node = bfs(avl->root->left->left); // find deepest node
                    } else if(left_height == (right_height - 2)){ /* right has more nodes */
                        if(avl->root->left->right)
                            new_node = bfs(avl->root->left->right); // find deepest node
                    }
                }
            }
        }
    }

    return new_node;
}

static Node *bfs(Node *start_node){
    Node *node = start_node;
    Node *ret = NULL;
    Rbuf rbuf;

    rbuf_init(&rbuf);
    rbuf_push(&rbuf, start_node);
    while(!rbuf_isempty(&rbuf)){
        ret = rbuf_pop(&rbuf);

        if(node->left)
            rbuf_push(&rbuf, node->left);

        if(node->right)
            rbuf_push(&rbuf, node->right);

        node = rbuf.buf[rbuf.head];
    }
    rbuf_destruct(&rbuf);

    return ret;
}

static int avl_balance(AVL *avl, Node *new_node, int case_type){
    switch(case_type){
        case LL:
            printf("insert %d is case LL\n", new_node->val);
            avl_ll_rot(avl, new_node->parent);
            break;

        case RR:
            printf("insert %d is case RR\n", new_node->val);
            avl_rr_rot(avl, new_node->parent);
            break;

        case LR:
            printf("insert %d is case LR\n", new_node->val);
            avl_lr_rot(avl, new_node);
            break;

        case RL:
            printf("insert %d is case RL\n", new_node->val);
            avl_rl_rot(avl, new_node);
            break;

        default: /* should not reach here */
            break;
    }

    return 0;
}
