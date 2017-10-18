#include <stdio.h>
#include <stdlib.h>

typedef struct node
{
    void *value;
    struct node *right;
    struct node *left;
} NODE;

NODE* create_node(void *value);
void print_int_tree(NODE* n,int);
void free_tree(NODE* n);
NODE* insert_node(NODE *tree,void * new_item,int (*f)(void *,void*));
int save_tree(FILE* destiny,NODE *tree,int size);
NODE * mount_tree(FILE* origin,int (*f)(void *,void*),int size);
