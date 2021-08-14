#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HASHMAP_DFLT_CAP_BITS 2u 
#define LOAD_FACTOR 0.75

typedef int (*fptr_cmp) (const void *, const void *);

typedef struct entry {
    char *term;
    int cnt;
} Entry; 

typedef struct map {
    Entry *buckets;
    size_t capacity;
    size_t size;
    double load_factor; 
    Entry *entries;
} HashMap;

size_t hash33(const char *term);

size_t map_indexer(HashMap *map, const char *term);

int map_init(HashMap *map);

int map_insert(HashMap *map, const char *term, size_t idx);

int map_delete(HashMap *map, const char *term);

size_t map_linear_prob(HashMap *map, size_t idx);

int map_find(HashMap *map, const char *term, const int inc_mode);

int map_rehash(HashMap *map);

int map_entries(HashMap *map);

int map_destruct(HashMap *map);

int map_rehash(HashMap *map);

void map_sort(HashMap *map, fptr_cmp cmp);

int map_print(HashMap *map);

int asc_cmp(const void *a, const void *b); 

int desc_cmp(const void *a, const void *b);

int main(){
    char buf[0x0400];

    HashMap map;
    map_init(&map);

    while(fgets(buf, 0x0401, stdin)){
	buf[strcspn(buf, "\r\n")] = '\0';
	const int inc_mode = (buf[0] != '-');
	char *term = (inc_mode) ? buf : buf + 1;

	map_find(&map, term, inc_mode);
    }

    map_sort(&map, desc_cmp);
    map_print(&map);
    map_destruct(&map);

    return 0;
}

int map_init(HashMap *map){
    size_t cap = 1u << HASHMAP_DFLT_CAP_BITS;
    if(!(map->buckets = malloc(sizeof(Entry) * cap))) return -1;

    map->capacity = cap;
    map->size = 0;
    map->load_factor = 0.0;
    map->entries = NULL;

    for(size_t i = 0; i < map->capacity; ++i)
      map->buckets[i].term = NULL;

    return 0;
}

int map_find(HashMap *map, const char *term, const int inc_mode){
    if(map->load_factor >= LOAD_FACTOR)
      map_rehash(map);

    size_t idx = map_indexer(map, term);
    char *curr_term = map->buckets[idx].term;

    if(!curr_term){
	map_insert(map, term, idx);

	return 0;
    }

    if(!strcmp(curr_term, term)){
	if(inc_mode)
	  map->buckets[idx].cnt++;
	else
	  map->buckets[idx].cnt--;

	return 0;
    }

    for(size_t i = idx + 1; i != idx; i = (i + 1) & (map->capacity - 1)){
	if(i < map->capacity && !map->buckets[i].term){
	  map_insert(map, term, i);

	  return 0;
	}

	if(map->buckets[i].term && !strcmp(map->buckets[i].term, term)){
	    if(inc_mode)
	      map->buckets[i].cnt++;
	    else
	      map->buckets[i].cnt--;

	    return 0;
	}
    }

    return -1;
}

int map_insert(HashMap *map, const char *term, size_t idx){ 
    if(!(map->buckets[idx].term = strdup(term))) return -1;
    map->buckets[idx].cnt = 1;

    map->size++;
    map->load_factor = (double)map->size / (double)map->capacity;

    return 0;
}

int map_rehash(HashMap *map){
    Entry *old_buckets = map->buckets;
    size_t old_cap = map->capacity;

    size_t new_cap = map->capacity << 1u;
    Entry *new_buckets = malloc(sizeof(Entry) * new_cap);
    if(!new_buckets) return -1;

    map->buckets = new_buckets;
    map->capacity = new_cap;
    map->load_factor = (double)map->size / (double)map->capacity;

    for(size_t i = 0; i < old_cap; ++i){
        char *term = old_buckets[i].term;
	int cnt = old_buckets[i].cnt;

        if(term){
	    size_t idx = map_indexer(map, term);
	    if(!map->buckets[idx].term)
		goto move;

	    idx = map_linear_prob(map, idx);

	  move:
	    map->buckets[idx].term = term;
	    map->buckets[idx].cnt = cnt;
        }
    }
    free(old_buckets);

    return 0;
}

int map_destruct(HashMap *map){
    for(size_t i = 0; i < map->size; ++i){
	free(map->entries[i].term);
	map->entries[i].term = NULL;
    }

    free(map->buckets);
    free(map->entries);

    return 0;
}

int map_delete(HashMap *map, const char *term){
    size_t idx = map_indexer(map, term);

    if(!map->buckets[idx].term)
	return -1;

    if(map->buckets[idx].term && !strcmp(map->buckets[idx].term, term)){
        free(map->buckets[idx].term);
	/* map->buckets[idx].term not set to NULL due to find function */

        map->size--;
        map->load_factor = (double)map->size / (double)map->capacity;

        return 0;
    }

    for(size_t i = idx + 1; i != idx; i = (i + 1) & (map->capacity - 1)){
        if(map->buckets[i].term && !strcmp(map->buckets[i].term, term)){
            free(map->buckets[i].term);
	    /* map->buckets[idx].term not set to NULL due to find function */

            map->size--;
            map->load_factor = (double)map->size / (double)map->capacity;

            return 0;
        }
    } 

    return -1;
}


void map_sort(HashMap *map, fptr_cmp cmp){
    map_entries(map);

    qsort(map->entries, map->size, sizeof(Entry), cmp);
}

int map_print(HashMap *map){
    for(size_t i = 0; i < map->size; ++i){
      char *term = map->entries[i].term;
      int cnt = map->entries[i].cnt;
      printf("%d %s\n", cnt, term);
    } 

    return 0;
}

int map_entries(HashMap *map){
    map->entries = malloc(sizeof(Entry) * map->size);
    if(!map->entries) return -1;
    size_t idx = 0;

    for(size_t i = 0; i < map->capacity; ++i){
      char *term = map->buckets[i].term;
      int cnt = map->buckets[i].cnt;
      if(term){
	 map->entries[idx].term = term;
	 map->entries[idx++].cnt = cnt; 
      }
    } 

    return 0;
}

size_t hash33(const char *term){
    unsigned long hash = 5381;
    int c;

    while ((c = *term++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

size_t map_indexer(HashMap *map, const char *term){
    size_t hval = hash33(term);

    return hval & (map->capacity - 1);
}

int asc_cmp(const void *a, const void *b){
    const Entry e1 = *(const Entry *)a;
    const Entry e2 = *(const Entry *)b;

    return (e1.cnt == e2.cnt) ? (strcmp(e1.term, e2.term)) : (e1.cnt - e2.cnt);
}

int desc_cmp(const void *a, const void *b){
    const Entry e1 = *(const Entry *)a;
    const Entry e2 = *(const Entry *)b;

    return (e1.cnt == e2.cnt) ? (strcmp(e1.term, e2.term)) : (e2.cnt - e1.cnt);
}

size_t map_linear_prob(HashMap *map, size_t idx){
  for(size_t i = idx + 1; i != idx; i = (i + 1) & (map->capacity - 1)){
    if(i < map->capacity && !map->buckets[i].term){
	return i;
    }
  }

  return 0;
}
