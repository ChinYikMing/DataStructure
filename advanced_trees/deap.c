/* used 0 to indicate slot is empty, this is not so good, FIXME */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>

#define err_exit(msg) \
    do { \
        perror(msg); \
        exit(EXIT_FAILURE); \
    } while(0)

/* find log base 2, note: math.h has log2() */
#define clz(x) ({ \
    unsigned int ret; \
    __auto_type num = (x); \
    const size_t num_size = sizeof(num); \
    if (num) { \
        if (num_size == sizeof(int)) \
            ret = __builtin_clz(num); \
        else if(num_size == sizeof(long)) \
            ret = __builtin_clzl(num); \
        else if(num_size == sizeof(long long)) \
            ret = __builtin_clzll(num); \
        else if(num_size < sizeof(int)) \
            ret = __builtin_clz(num) - ((sizeof(int) - num_size) * CHAR_BIT); \
    } else { \
        ret = CHAR_BIT * num_size; \
    }\
    ret;\
})

#define fls(x) ({ \
    (CHAR_BIT * sizeof(typeof(x)) - clz(x)); \
})

#define lg2(x) ({ \
    __auto_type ret = (x); \
    ret ? fls(x) - 1 : 0; \
})

#define min(x, y) ({ \
    __auto_type _x = (x); \
    __auto_type _y = (y); \
    _x < _y ? _x : _y; \
})

#define max(x, y) ({ \
    __auto_type _x = (x); \
    __auto_type _y = (y); \
    _x > _y ? _x : _y; \
})

/**
 * deap->arr[0](root) is empty
 * deap->arr[0 * 2 + 1](left) is min heap
 * deap->arr[0 * 2 + 2](right) is max heap
 * deap[i] <= deap[j], i is index of min heap, j is index of max heap
 * how to find level of index i? Using i_level = lg2(i + 1)
 * how to find j from i? Using this formula: j = i + 2^(i_level - 2) => j = i + 2^(lg2(i + 1) - 2)
 */  

/* todo using struct node * replace simple array to avoid last slot is unused */
typedef struct deap {
    int *arr;          /* simple array, 0 means empty slot, so cannot store 0(FIXME) */
    int *j;            /* corresponding j */
    size_t size;
    size_t capacity;   /* power of 2, so two slot is empty which are first slot(root) and last slot since complete binary tree can have maximum 2^k - 1 slot */
    size_t level;
} Deap;

enum {
    MIN_HEAP,
    MAX_HEAP,
    NOR_HEAP     /* normal mode: choose the empty slot from min heap then max heap */
};

int deap_init(Deap *deap);
int deap_insert(Deap *deap, int val, int which_heap);
int deap_insert_max(Deap *deap, int val); /* insert to max heap */
int deap_insert_min(Deap *deap, int val); /* insert to min heap */
int deap_find_empty(Deap *deap, int *which_heap); /* return index of empty slot */
int deap_heapify_max(Deap *deap, int start);
int deap_heapify_min(Deap *deap, int start);
int deap_delete_min(Deap *deap);
int deap_delete_max(Deap *deap);
int deap_destroy(Deap *deap);
int deap_print(Deap *deap);
int deap_bfs(Deap *deap, int start, int *empty_idx);

int swap(int *a, int *b);
int find_j_by_i(int i);

#define	    RBUF_DFT_CAPACITY	8u
typedef struct rbuf {
  int *buf;
  size_t capacity;
  size_t size;
  size_t head;
  size_t tail;
} Rbuf;

int rbuf_init(Rbuf *rbuf);
int rbuf_destruct(Rbuf *rbuf);
int rbuf_push(Rbuf *rbuf, int idx);
int rbuf_pop(Rbuf *rbuf);
int rbuf_isfull(Rbuf *rbuf);
int rbuf_isempty(Rbuf *rbuf);

int main(){
    Deap deap;
    deap_init(&deap);

    deap_insert(&deap, 1, NOR_HEAP);
    deap_insert(&deap, 90, NOR_HEAP);
    deap_insert(&deap, 3, NOR_HEAP);
    deap_insert(&deap, 5, NOR_HEAP);
    deap_insert(&deap, 40, NOR_HEAP);
    deap_insert(&deap, 80, NOR_HEAP);
    deap_insert(&deap, 10, NOR_HEAP);
    deap_insert(&deap, 11, NOR_HEAP);
    deap_insert(&deap, 17, NOR_HEAP);
    deap_insert(&deap, 16, NOR_HEAP);
    deap_insert(&deap, 30, NOR_HEAP);
    deap_insert(&deap, 20, NOR_HEAP);
    deap_insert(&deap, 70, NOR_HEAP);
    deap_insert(&deap, 60, NOR_HEAP);
    deap_print(&deap);

    int min = deap_delete_min(&deap);
    printf("min: %d\n", min);
    deap_print(&deap);
    min = deap_delete_min(&deap);
    printf("min: %d\n", min);
    deap_print(&deap);
    
    int max = deap_delete_max(&deap);
    printf("max: %d\n", max);
    deap_print(&deap);
    max = deap_delete_max(&deap);
    printf("max: %d\n", max);
    deap_print(&deap);

    deap_destroy(&deap);
    exit(EXIT_SUCCESS);
}

int deap_init(Deap *deap){

    /**  initial 
     *      a(root)
     *     / \
     *    b   c
     */
    size_t cap = 1 << 2;
    deap->arr = malloc(sizeof(int) * cap);
    if(!deap->arr)
        err_exit("init arr failed");

    deap->j = malloc(sizeof(int) * cap);
    if(!deap->arr)
        err_exit("init j failed");
    
    memset(deap->arr, 0, sizeof(int) * cap);
    deap->size = 0; 
    deap->capacity = cap;
    deap->level = 2; /* depends on cap */
    return 0;
}

int deap_insert(Deap *deap, int val, int which_heap){
    if(deap->capacity - 2 == deap->size){ /* full, expands 2x */
        size_t cap_new = deap->capacity << 1;
        void *tmp = realloc(deap->arr, sizeof(int) * cap_new);
        if(!tmp)
            err_exit("realloc arr failed");

        deap->arr = tmp;
        deap->capacity = cap_new;
        deap->level++;

        tmp = realloc(deap->j, sizeof(int) * cap_new);
        if(!tmp)
            err_exit("realloc j failed");
        deap->j = tmp;
    }

    int i;                                  /* last empty slot index */
    int j;                                  /* corresponding slot j to slot i*/
    int *d = deap->arr;
    int *corr_j = deap->j;

    i = deap_find_empty(deap, &which_heap); /* i is index of min heap */
    d[i] = val;
    deap->size++;

    if(which_heap == MIN_HEAP){              /* i in min heap */
        j = corr_j[i] ? corr_j[i] : find_j_by_i(i);
        deap->j[j] = i;                      /* (1) for max heap find correspond j in min heap */
        j = d[j] == 0 ? (j - 1) / 2 : j;     /* if j is empty, then compare to parent */
        if(d[j] != 0 && d[i] > d[j]){
            swap(&d[i], &d[j]);
            deap_heapify_max(deap, j);
        } else
            deap_heapify_min(deap, i);
    } else {                                 /* i in max heap */
        j = deap->j[i];                      /* (1) */
        if(d[j] != 0 && d[i] < d[j]){
            swap(&d[i], &d[j]);
            deap_heapify_min(deap, j);
        } else
            deap_heapify_max(deap, i);
    }

    return 0;
}

int deap_insert_max(Deap *deap, int val){
    return deap_insert(deap, val, MAX_HEAP);
}

int deap_insert_min(Deap *deap, int val){
    return deap_insert(deap, val, MIN_HEAP);
}

int deap_heapify_max(Deap *deap, int start){
    int idx_parent = (start - 1) / 2;
    int *d = deap->arr;

    if(idx_parent == 0 /* reach root */ || d[start] < d[idx_parent])
        return 0;

    swap(&d[start], &d[idx_parent]);
    deap_heapify_max(deap, idx_parent);
}

int deap_heapify_min(Deap *deap, int start){
    int idx_parent = (start - 1) / 2;
    int *d = deap->arr;

    if(idx_parent == 0 /* reach root */ || d[start] > d[idx_parent])
        return 0;

    swap(&d[start], &d[idx_parent]);
    deap_heapify_min(deap, idx_parent);
}

int deap_delete_min(Deap *deap){
    if(deap->size == 0)
        return 0;

    int *d = deap->arr;
    int min_idx = 1;           /* root of min heap */
    int min = d[min_idx];
    int left_idx, right_idx;
    int next_min, next_min_idx; 

    int max_idx_in_level = min_idx;
    int deap_level_from_child = deap->level - 1; /* -1 for ignoring root level */
    if(deap_level_from_child >= 2){
        int time_to_find_max_idx = deap_level_from_child - 1; /* e.g., level 3 complete binary tree has to do 2 times of 2 * i + 1 */
        for(int i = 0; i < time_to_find_max_idx; ++i)
            max_idx_in_level = (max_idx_in_level << 1) + 2;
    }

    /* assume left and right have value(not 0) */
again:
    left_idx = (min_idx << 1) + 1;
    right_idx = (min_idx << 1) + 2;

    if(left_idx > max_idx_in_level || right_idx > max_idx_in_level ||  /* reach leaf */
            (d[left_idx] == 0 && d[right_idx] == 0) /* has that level, but both empty */){
            d[min_idx] = 0;
            goto end;
    }

    next_min = min(d[left_idx], d[right_idx]);
    next_min_idx = (next_min == d[left_idx] ? left_idx : right_idx);
    d[min_idx] = next_min;
    d[next_min_idx] = 0;
    min_idx = next_min_idx;
    goto again;

end:
    if(d[deap->size] != 0)                        /* next min is the child, and the child will set to 0, so we do not have insert it again */
        deap_insert_min(deap, d[deap->size--]);   /* insert last to the leaf to fulfil complete binary tree property */
    d[deap->size--] = 0;                          /* 0 means empty */

    return min;
}

int deap_delete_max(Deap *deap){
    if(deap->size == 0)
        return 0;

    int *d = deap->arr;
    int max_idx = 2;           /* root of max heap */
    int max = d[max_idx];
    int left_idx, right_idx;
    int next_max, next_max_idx; 

    int max_idx_in_level = max_idx;
    int deap_level_from_child = deap->level - 1; /* -1 for ignoring root level */
    if(deap_level_from_child >= 2){
        int time_to_find_max_idx = deap_level_from_child - 1; /* e.g., level 3 complete binary tree has to do 2 times of 2 * i + 1 */
        for(int i = 0; i < time_to_find_max_idx; ++i)
            max_idx_in_level = (max_idx_in_level << 1) + 2;
    }

    /* assume left and right have value(not 0), FIXME */
again:
    left_idx = (max_idx << 1) + 1;
    right_idx = (max_idx << 1) + 2;

    if(left_idx > max_idx_in_level || right_idx > max_idx_in_level || /* reach leaf */
            (d[left_idx] == 0 && d[right_idx] == 0) /* has that level, but both empty */){
            d[max_idx] = 0;
            goto end;
    }

    next_max = max(d[left_idx], d[right_idx]);
    next_max_idx = (next_max == d[left_idx] ? left_idx : right_idx);
    d[max_idx] = next_max;
    d[next_max_idx] = 0;
    max_idx = next_max_idx;
    goto again;

end:
    if(d[deap->size] != 0)                      /* next max is the child, and the child will set to 0, so we do not have insert it again */
        deap_insert_max(deap, d[deap->size--]); /* insert last to the leaf to fulfil complete binary tree property */
    d[deap->size--] = 0;                        /* 0 means empty */

    return max;
}

int deap_destroy(Deap *deap){
    free(deap->arr);
    free(deap->j);
    return 0;
}

int swap(int *a, int *b){
    if(!a || !b)
        return 0;

    int tmp = *a;
    *a = *b;
    *b = tmp;
    return 0;
}

/* how to find level of index i? Using i_level = hi(lg2(i + 1)) = lg2(i + 1) + 1 */
/* how to find j from i? Using this formula: j = i + 2^(i_level - 2) => j = i + 2^(lg2(i + 1) - 2) */
int find_j_by_i(int i){
    int i_level = lg2(i + 1) + 1; 
    int j = i + (int) pow(2, ((double) i_level) - 2.0);
    return j;
}

int deap_print(Deap *deap){
    if(deap->size == 0){
        printf("empty\n");
        return 0;
    }

    for(int i = 0; i < deap->capacity; ++i)
        printf("%d\n", deap->arr[i]);
    return 0;
}

int deap_find_empty(Deap *deap, int *which_heap){
    int *d = deap->arr;
    int found = 0;
    int empty_idx;
    if(*which_heap == MAX_HEAP)         /* caller guarantee max heap has empty slot */
        deap_bfs(deap, 2, &empty_idx);
    else if(*which_heap == MIN_HEAP)    /* caller guarantee min heap has empty slot */
        deap_bfs(deap, 1, &empty_idx);
    else { /* normal mode */
        /* bfs find the min heap(deap->arr[1]), if found then next empty slot is in min heap else in max heap*/
        found = deap_bfs(deap, 1, &empty_idx);
        if(found)
            *which_heap = MIN_HEAP;
        else {
            deap_bfs(deap, 2, &empty_idx); /* bfs max heap */
            *which_heap = MAX_HEAP;
        }
    }

    return empty_idx;
}

int rbuf_init(Rbuf *rbuf){
   rbuf->buf = malloc(sizeof(int) * RBUF_DFT_CAPACITY);
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

int rbuf_push(Rbuf *rbuf, int idx){
    if(rbuf_isfull(rbuf)){
	size_t new_capacity = rbuf->capacity << 1u;
	size_t val_cnt_after_head = rbuf->capacity - rbuf->head;
	void *tmp = realloc(rbuf->buf, sizeof(int) * new_capacity);
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
    rbuf->buf[rbuf->tail] = idx;
    rbuf->tail = (rbuf->tail + 1) % rbuf->capacity;    
    rbuf->size++;

    return 0;
}

int rbuf_pop(Rbuf *rbuf){
    if(rbuf_isempty(rbuf))
        err_exit("The ring buffer is empty");

    // invariant: the rbuf has entities
    int pop_val = rbuf->buf[rbuf->head];
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

int deap_bfs(Deap *deap, int start, int *empty_idx){
    int idx;
    int ret;
    int *d = deap->arr;
    int max_idx = start;
    int deap_level_from_child = deap->level - 1; /* -1 for ignoring root level */
    if(deap_level_from_child >= 2){
        int time_to_find_max_idx = deap_level_from_child - 1; /* e.g., level 3 complete binary tree has to do 2 times of 2 * i + 1 */
        for(int i = 0; i < time_to_find_max_idx; ++i)
            max_idx = (max_idx << 1) + 2;
    }

    Rbuf rbuf;
    rbuf_init(&rbuf);
    rbuf_push(&rbuf, start);
    while(!rbuf_isempty(&rbuf)){
        idx = rbuf_pop(&rbuf);

        if(d[idx] == 0){       /* 0 means empty */
            *empty_idx = idx;
            goto end;
        } else {
            rbuf_push(&rbuf, (idx << 1) + 1); /* index of left is 2 * i + 1*/
            rbuf_push(&rbuf, (idx << 1) + 2); /* index of right is 2 * i + 2*/
        }

        if(idx == max_idx)
            break;
    }

    rbuf_destruct(&rbuf);
    return 0; /* not found */

end:
    rbuf_destruct(&rbuf);
    return 1;
}
