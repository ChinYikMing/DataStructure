#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF_SIZE 1024

#define is_empty(list) ( NULL == *list ) 

#define node_for(list, node)  \
    for(node = *list; NULL != node; node = node->next)

typedef struct user {
    char *name;
} User; 

typedef struct node Node;
struct node {
    void *data;
    Node *next;   
};

void usage();
void node_insert_front(Node **list, char *name);
void node_insert_tail(Node **list, Node *node, char *name);
void node_delete(Node **list, Node *node, char *name);
void node_update(Node **list, Node *node, User *u, char *name, char *new);
void node_find(Node **list, Node *node, User *u, char *name);
void node_print(Node **list, Node *node, User *u); 

int main(){
    Node *list = NULL;
    Node *node = NULL;
    User *u = NULL;

    char buf[BUF_SIZE];
    char cmd;
    char *last_space_position;
    size_t offset;
    char name[16];
    char new[16];

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
	} else 
	    strcpy(name, buf + 2);

	switch(cmd){
	    case 'i':
	      node_insert_front(&list, name);
	      break;

	    case 't':
	      node_insert_tail(&list, node, name);
	      break;

	    case 'd':
	      node_delete(&list, node, name);
	      break;

	    case 'u':
	      node_update(&list, node, u, name, new);
	      break;

	    case 'f':
	      node_find(&list, node, u, name); 
	      break;

	    case 'p':
	      node_print(&list, node, u);
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
   printf("The format is 'cmd: name node', node[OPTIONAL](for updating current node)\n\n"); 
   printf("cmd: 'i'(insert node at front)\t't'(insert node at tail)\t'd'(delete node)\t'f'(find all the node having the pattern)\t'p'(print all the node info)\t'h'(usage)\n");
}

void node_insert_front(Node **list, char *name){
    size_t name_len = strlen(name);

    Node *node = malloc(sizeof(Node));
    User *user = malloc(sizeof(User));
    user->name = malloc(name_len + 1);
    node->data = user;
    memcpy(user->name, name, name_len + 1);

    node->next = (is_empty(list)) ? NULL : *list;

    *list = node;
}

void node_insert_tail(Node **list, Node *node, char *name){
    node = *list;
    size_t name_len = strlen(name);

    Node *new_node = malloc(sizeof(Node));
    User *new_user = malloc(sizeof(User));
    new_user->name = malloc(name_len + 1);
    new_node->next = NULL;
    new_node->data = new_user;
    memcpy(new_user->name, name, name_len + 1);

    if(is_empty(list)){
	*list = new_node;
	return;
    }

    while(node->next)
	node = node->next;

    node->next = new_node;
}

void node_delete(Node **list, Node *node, char *name){
    node = *list;
    Node *prev;
    User *u = node->data; 

    /* case of remove the first node */
    if(!strcmp(u->name, name)){
	*list = (*list)->next;
	return;
    } 

    prev = node;
    node = node->next;

    while(node){
	u = node->data;
	if(!strcmp(u->name, name)){
	    prev->next = node->next;
	    free(u->name);
	    free(node);
	    return;
	}
	prev = node;
	node = node->next;
    }
}

void node_print(Node **list, Node *node, User *u){ 
    node_for(list, node){
      u = node->data;
      printf("%s\n", u->name);
    }
}

void node_find(Node **list, Node *node, User *u, char *name){ 
    node_for(list, node){
	u = node->data; 
	if(strstr(u->name, name))
	  printf("Found: %s having pattern '%s'\n", u->name, name); 
    }
}

void node_update(Node **list, Node *node, User *u, char *name, char *new){ 
    node_for(list, node){
	u = node->data; 
	if(!strcmp(u->name, name)){ 
	    strcpy(u->name, new); 
	    return; 
	} 
    }
}
