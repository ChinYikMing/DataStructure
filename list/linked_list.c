#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define BUF_SIZE 1024

typedef struct user {
    char *name;
} User;

typedef struct node Node;
struct node {
    void *data;
    Node *next;   
};

void usage();
Node *node_insert_front(Node *curr, char *name);
Node *node_insert_tail(Node *curr, char *name);
Node *node_delete(Node *curr, char *name);
Node *node_update(Node *curr, char *name, char *new);
Node *node_find(Node *curr, char *pattern);
void node_print(Node *curr);

int main(){
    Node *curr = NULL;
    char buf[BUF_SIZE];
    char cmd;
    char name[16];
    char new[16];
    char *last_space_position;
    size_t offset;

    while(fgets(buf, BUF_SIZE, stdin)){
	/* remove new line */
	{
	    size_t buf_len = strlen(buf);
	    buf[buf_len - 1] = '\0';
	}
	cmd = buf[0];

	if('u' == cmd){
	    last_space_position = strrchr(buf, ' ');
	    offset = last_space_position - (buf + 2);
	    memcpy(name, buf + 2, offset + 1);
	    name[offset] = '\0';

	    strcpy(new, buf + offset + 3);
	} else {
	    strcpy(name, buf + 2);
	}

	switch(cmd){
	    case 'i':
	      curr = node_insert_front(curr, name);
	      break;

	    case 't':
	      curr = node_insert_tail(curr, name);
	      break;

	    case 'd':
	      curr = node_delete(curr,name);
	      break;

	    case 'u':
	      curr = node_update(curr, name, new);
	      break;

	    case 'f':
	      curr = node_find(curr, name); 
	      break;

	    case 'p':
	      node_print(curr);
	      break;

	    case 'h':
	      usage();
	      break;

	    default:
	      usage();
	      break;
	}
    }
}

void usage(){
   printf("The format is 'cmd: name new', new[OPTIONAL](for updating current node)\n\n"); 
   printf("cmd: 'i'(insert node at front)\t't'(insert node at tail)\t'd'(delete node)\t'f'(find all the node having the pattern)\t'p'(print all the node info)\t'h'(usage)\n");
}
Node *node_insert_front(Node *curr, char *name){
    size_t name_len = strlen(name);
    if(!curr && isalpha(*name)){
	Node *new = malloc(sizeof(Node));
	User *user = malloc(sizeof(User));
	user->name = malloc(name_len + 1);
	strcpy(user->name, name);
	
	new->data = user;
	new->next = NULL;
	return new;
    } else if(isalpha(*name)){
	Node *new = malloc(sizeof(Node));

	User *user = malloc(sizeof(User));
	user->name = malloc(name_len + 1);
	strcpy(user->name, name);

	Node *prev;
	prev = curr;
	new->data = user;
	new->next = prev;
	return new;
    }
    return curr;
}

Node *node_insert_tail(Node *curr, char *name){
    size_t name_len = strlen(name);
    if(!curr && isalpha(*name)){
	Node *new = malloc(sizeof(Node));
	User *user = malloc(sizeof(User));
	user->name = malloc(name_len + 1);
	strcpy(user->name, name);
	
	new->data = user;
	new->next = NULL;
	return new;
    } else if(isalpha(*name)){
	Node *new = malloc(sizeof(Node));
	Node *prev;

	User *user = malloc(sizeof(User));
	user->name = malloc(name_len + 1);
	strcpy(user->name, name);

	new->data = user;
	new->next = NULL;

	prev = curr;
	while(prev->next)
	    prev = prev->next;

	prev->next = new;
    }
    return curr;
}

Node *node_delete(Node *curr, char *name){
    User *user = curr->data;
    Node *p, *prev;

    /* case of romove the first node */
    if(!strcmp(user->name, name)){
	p = curr->next;
	return p;
    } 

    prev = curr;
    p = curr->next;

    while(p){
	user = p->data;
	if(!strcmp(user->name, name)){
	    prev->next = p->next;
	    free(user->name);
	    free(p);
	    return curr;
	}
	prev = p;
	p = p->next;
    }
    return curr;
}

Node *node_update(Node *curr, char *name, char *new){
    Node *p = curr;
    while(p){
	User *user = p->data;
	if(!strcmp(user->name, name)){
	    strcpy(user->name, new);
	    return curr;
	}
	p = p->next;
    }
    return curr;
}

Node *node_find(Node *curr, char *pattern){
    Node *p = curr;
    while(p){
	User *user = p->data;

	if(strstr(user->name, pattern))
	    printf("Found: %s having pattern '%s'\n", user->name, pattern);

	p = p->next;
    }
    return curr;
}

void node_print(Node *curr){
    while(curr){
	User *user = curr->data;
	printf("%s\n", user->name);
	curr = curr->next;
    }
}
