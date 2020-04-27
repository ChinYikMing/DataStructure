#include    <stdio.h>
#include    <stdlib.h> 
#include    <string.h>
#include    "bst.h"
#include    "ring_buf.h"

int main(){
    BST *root = NULL;
    BST_init(&root);

    BST_insert(&root, 30);
    BST_insert(&root, 40);

    BST_insert(&root, 28);

    BST_insert(&root, 48);

    BST_insert(&root, 50);
    BST_insert(&root, 45);

    BST_insert(&root, 20);
    BST_insert(&root, 36);

    BST_insert(&root, 10);
    BST_insert(&root, 22);

    BST_insert(&root, 5);
    BST_insert(&root, 12);
    BST_insert(&root, 15);

    BST_insert(&root, 1);
    BST_insert(&root, 8);

    BST_delete(&root, 30);

    printf("level traversal:\n");
    BST_level_traversal(&root);

    printf("preorder traversal:\n");
    BST_pre_order_traversal(&root);

    printf("inorder traversal:\n");
    BST_in_order_traversal(&root);

    printf("postorder traversal:\n");
    BST_post_order_traversal(&root);

    BST_destruct(&root);

    return 0;
}
