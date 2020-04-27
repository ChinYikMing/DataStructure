#ifndef	    BST_H
#define	    BST_H

#define	    ERR_DATA_EXISTS	-2
#define	    ERR_DATA_NOT_FOUND	-3

typedef struct bst BST;
struct bst {
    BST *left_node;
    BST *right_node;
    int data;
    _Bool deleted;
};

void BST_init(BST **root);

BST *BST_node_new(int data);

int BST_insert(BST **root, int data);

int BST_delete(BST **root, int data);

void BST_destruct(BST **root);

void BST_pre_order_traversal(BST **root);

void BST_post_order_traversal(BST **root);

void BST_in_order_traversal(BST **root);

void BST_level_traversal(BST **root);

#endif
