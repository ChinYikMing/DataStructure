#include <stdio.h>
#include <string.h>
#include <stdlib.h>
 
#define BUF_SIZE 1024
#define MAP_CAP_BITS 5u

typedef struct entry {
    char *key;
    int value;
    struct entry *next;
} Entry;
 
typedef struct {
    Entry *buckets; 
    size_t capacity;
    size_t size; 
} HashMap;
 
 
int map_init(HashMap *map, unsigned int cap_bits);
 
int map_entries(HashMap *map, Entry ***entries);
 
int entry_cmp(const void *a, const void *b);
 
size_t hash33(const char *key);
 
size_t map_idx(HashMap *map, const char *key);
 
int map_put(HashMap *map, const char *key, int value);
 
int *map_get(HashMap *map, const char *key);
 
void map_destroy(HashMap *map);
 
int main() {
    HashMap map;
    if (map_init(&map, MAP_CAP_BITS) < 0)
        fprintf(stderr, "map_init error\n");
 
    char buf[BUF_SIZE];
    while (fgets(buf, BUF_SIZE, stdin)) {
        buf[strcspn(buf, "\r\n")] = '\0';
 
        const _Bool decrease = (*buf == '-');
 
        char *term;
        int increment;
 
        if (decrease) {
            term = buf + 1;
            increment = -1;
        } else {
            term = buf;
            increment = 1;
        }
 
        int *value = map_get(&map, term);
 
        if (value)
            (*value) += increment;
        else if (!decrease)
            map_put(&map, term, 1);
    }
 
    /*      OUTPUT     */
    {
        const size_t size = map.size;
        Entry **entries;
        if (map_entries(&map, &entries) < 0)
            fprintf(stderr, "map_entries error\n");
 
        qsort(entries, size, sizeof(Entry *), entry_cmp);
 
        for (size_t i = 0; i < size; i++) {
            Entry *e = entries[i];
            const char *term = e->key;
            const int count = e->value;
            printf("%d %s\n", count, term);
        } 
        free(entries);
    }
 
    map_destroy(&map);
}
 
int map_init(HashMap *map, unsigned int cap_bits) {
    const size_t capacity = 1u << cap_bits; // 計算 2 的 cap_bts 次方
    map->buckets = calloc(capacity, sizeof(Entry)); // 分配「全為 0」的空間
    map->capacity = capacity;
    map->size = 0;
 
    return -(map->buckets == NULL);
}
 
int entry_cmp(const void *a, const void *b){
    const Entry *e1 = *(const Entry **)a; 
    const Entry *e2 = *(const Entry **)b; 

    return (e1->value == e2->value) ? (strcmp(e1->key, e2->key)) : (e2->value - e1->value);
}
 
size_t hash33(const char *key){
    unsigned long hval = 5381;
    char c;

    while((c = *key++))
      hval = ((hval << 5u) + hval) + c;

    return hval;
}
 
size_t map_idx(HashMap *map, const char *key){
   size_t hval = hash33(key);

   return hval & (map->capacity - 1);
}
 
int map_put(HashMap *map, const char *key, int value){
    size_t idx = map_idx(map, key);

    if(!map->buckets[idx].key){
	if(!(map->buckets[idx].key = strdup(key))) return -1;
	map->buckets[idx].value = value;

	map->size++;
	return 0;	
    }

    if(map->buckets[idx].key){
	if(!strcmp(map->buckets[idx].key, key)){
	    map->buckets[idx].value = value;
	    return 0;
	}
	
	Entry **ptr = &map->buckets[idx].next;
	Entry *e;
	while((e = *ptr)){
	    if(!strcmp(e->key, key)){
		e->value = value;
		return 0;
	    }
	    ptr = &e->next;
	}

	if(!(*ptr = malloc(sizeof(Entry)))) return -1;
	if(!((*ptr)->key = strdup(key))) return -1;
	(*ptr)->value = value;
	(*ptr)->next = NULL;

	map->size++;
    }
    
    return 0;
}
 
int *map_get(HashMap *map, const char *key){
    size_t idx = map_idx(map, key);

    if(!map->buckets[idx].key)
	return NULL;	

    if(map->buckets[idx].key){
	if(!strcmp(map->buckets[idx].key, key))
	    return &map->buckets[idx].value;
	
	Entry *curr = map->buckets[idx].next;
	while(curr){
	    if(!strcmp(curr->key, key))
		return &curr->value;

	    curr = curr->next;
	}
    }

    return NULL;
}
 
void map_destroy(HashMap *map){
   for(size_t i = 0; i < map->capacity; ++i){
	Entry bucket = map->buckets[i];
	if(bucket.key){
	    Entry *curr, *next = bucket.next;
	    while(next){
		curr = next;
		next = next->next;
		
		free(curr->key);
		free(curr);
	    }
	    free(bucket.key);
	}
   } 

   free(map->buckets);
}

int map_entries(HashMap *map, Entry ***entries){
    if(map->size == 0){
	*entries = NULL;
	return 0;
    }

    Entry *table = map->buckets;

    Entry **ret = malloc(sizeof(Entry *) * map->size);
    if(!(*entries)) return -1;
    size_t idx = 0;

    for(size_t i = 0; i < map->capacity; ++i){
	Entry *e = table + i;
	if(e->key){
	  while(e){
	    ret[idx++] = e;
	    e = e->next;
	  }
	}
    }

    *entries = ret;
    return 0;
}
