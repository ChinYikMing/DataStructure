#include    <stdio.h>
#include    <stdlib.h>
#include    "bst.h"
#include    "ring_buf.h"

void BST_init(BST **root){
    *root = BST_node_new(25);
    return;
}

BST *BST_node_new(int data){
    BST *new_node = malloc(sizeof(BST));
    if(!new_node) return NULL;
    // invariant: new_node is successfully malloc
    new_node->data = data;
    new_node->left_node = NULL;
    new_node->right_node = NULL;
    new_node->deleted = 0;
    
    return new_node;
}

int BST_insert(BST **root, int data){
    BST **node = root;

    while(*node){
	if(data == (*node)->data)
	  return ERR_DATA_EXISTS;
	else if(data > (*node)->data)
	  node = &(*node)->right_node;
	else
	  node = &(*node)->left_node;
    }
    *node = BST_node_new(data);

    return 0;
}

int BST_delete(BST **root, int data){
   BST **node = root; 
    
   while(*node){
	if((*node)->data == data){
	    (*node)->deleted = 1;
	    return 0;
	}
	//invariant: the data is not in BST	
	if(data > (*node)->data)
	  node = &(*node)->right_node;
	else 
	  node = &(*node)->left_node;
   }
   
   return ERR_DATA_NOT_FOUND;
}

void BST_destruct(BST **root){
    BST *node = *root;
    if(!node) return;
    //invariant: node is not NULL
    BST_destruct(&node->left_node);
    BST_destruct(&node->right_node);
    free(node);
}

void BST_pre_order_traversal(BST **root){
    BST *node = *root;
    if(!node) return;
    // invariant: node is not NULL
    if(!node->deleted)
	printf("%d\n", node->data);
    BST_pre_order_traversal(&node->left_node);
    BST_pre_order_traversal(&node->right_node);
}

void BST_post_order_traversal(BST **root){
    BST *node = *root;
    if(!node) return;
    // invariant: node is not NULL
    BST_post_order_traversal(&(node->left_node));
    BST_post_order_traversal(&(node->right_node));
    if(!node->deleted)
        printf("%d\n", node->data);
}

void BST_in_order_traversal(BST **root){
    BST *node = *root;
    if(!node) return;
    // invariant: node is not NULL
    BST_in_order_traversal(&node->left_node);
    if(!node->deleted)
	 printf("%d\n", node->data);
    BST_in_order_traversal(&node->right_node);
}

void BST_level_traversal(BST **root){
    BST *node = *root;
    Rbuf rbuf;
    rbuf_init(&rbuf);
      
    rbuf_push(&rbuf, node);
    while(!rbuf_isempty(&rbuf)){
	if(node->deleted){
	    rbuf.size--;
	    rbuf.head = (rbuf.head + 1) % rbuf.capacity;
	} else
	    rbuf_pop(&rbuf);

	if(node->left_node)
	  rbuf_push(&rbuf, node->left_node);

	if(node->right_node)
	  rbuf_push(&rbuf, node->right_node);

	node = rbuf.buf[rbuf.head];
    }
    rbuf_destruct(&rbuf);

    return;
}
