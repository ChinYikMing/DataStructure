#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

typedef struct node Node;
struct node {
    void *data;
    Node *next;
    Node *prev;
};

typedef struct list {
    Node *head;
    size_t size;
} List;

typedef struct user {
    char *name;
    int id;
} User;

int list_init(List *list);
Node *node_create(void *data);
int list_push_back(List *list, void *data);
int list_push_front(List *list, void *data);
int list_delete(List *list, int id);

int main(){
    List list;
    list_init(&list);

    list_push_back(&list, &(User){
	.name = "yik ming",
	.id = 81	
    });

    list_push_back(&list, &(User){
	.name = "sing lek",
	.id = 11	
    });

    list_push_front(&list, &(User){
	.name = "Yc",
	.id = 12
    }); 

    list_push_front(&list, &(User){
	.name = "Jason",
	.id = 45
    });

    list_delete(&list, 12);
    list_delete(&list, 11);

    Node *ptr = list.head;
    while(ptr){
	User *u = ptr->data;
	printf("%s's id is %d\n", u->name, u->id);
	ptr = ptr->next;
	if(ptr == list.head) break;
    }
}

int list_init(List *list){
    list->head = NULL;
    list->size = 0;
    return 0;
}

Node *node_create(void *data){
    Node *new_node = malloc(sizeof(Node));
    new_node->data = data;
    new_node->next = NULL;
    new_node->prev = NULL;
    return new_node;
}

int list_push_back(List *list, void *data){
    Node *new_node = node_create(data);
    if(!new_node) return -1;

    list->size++;

    if(!list->head){
	list->head = new_node;
	return 0;
    }

    Node *ptr = list->head;
    while(ptr->next && ptr->next != list->head)
	ptr = ptr->next;

    ptr->next = new_node;
    new_node->prev = ptr; 

    /* implement circular linked list */
    new_node->next = list->head;
    list->head->prev = new_node;
    return 0;
}

int list_push_front(List *list, void *data){
    Node *new_node = node_create(data);
    if(!new_node) return -1;

    list->size++;

    if(!list->head){
	list->head = new_node;
	return 0;
    }

    /* find the last node to implement circular linked list */
    Node *ptr = list->head;
    while(ptr->next && ptr->next != list->head)
	ptr = ptr->next;

    ptr->next = new_node;
    new_node->prev = ptr;

    /* implement circular linked list */
    new_node->next = list->head;
    list->head->prev = new_node;
    list->head = new_node;
    return 0;
}

int list_delete(List *list, int id){
    Node *ptr = list->head;
    User *u = ptr->data;

    /* check the node which the list has only 1 node*/
    if(u->id == id && list->size == 1){
//	free(u->name);
	free(ptr);
	list->head = NULL;
	return 0;
    }
    
    /* check the first node which the list has at least 2 nodes*/
    if(u->id == id){
	Node *prev = ptr->prev;
	Node *next = ptr->next;
//	free(u->name);
	free(ptr);
	next->prev = prev;
	prev->next = next;
	list->head = next;
	list->size--;
	return 0;
    } 

    /* check the last node which the list has at least 2 nodes */
    ptr = ptr->prev;
    u = ptr->data;
    if(u->id == id){
	Node *prev = ptr->prev;
	Node *next = ptr->next;
//	free(u->name);
	free(ptr);
	prev->next = next;
	next->prev = prev;
	list->size--;
	return 0;
    }

    ptr = list->head;
    /* check the between nodes */
    while(ptr->next && ptr->next != list->head){
	u = ptr->data;
	if(u->id == id){
	    Node *prev = ptr->prev;
	    Node *next = ptr->next;
//	    free(u->name);
	    free(ptr);
	    prev->next = next;
	    next->prev = prev;
	    list->size--;
	    return 0;
	}
	ptr = ptr->next;
    }
    return -1;
}
