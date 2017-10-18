#include "tree.h"
NODE* create_node(void *value){
    NODE* new_node = (NODE *)malloc(sizeof(NODE));
    new_node->value = value;
    new_node->left=NULL;
    new_node->right=NULL;
    return new_node;
}

void print_int_tree(NODE* n,int depth){
    if(!n)
        return;
    int i=0;
    for(;i<depth;i++)
        printf("-");
    printf("%d\n",*(int*)n->value);
    print_int_tree(n->left,depth+1);
    print_int_tree(n->right,depth+1);
}

int free_node(NODE* n){

    if(!n)
        return 1;

    free(n->value);
    free(n);

    return 0;
}

void free_tree(NODE* n){
    if(n == NULL)
        return;
    NODE *l,*r;
    r = n->right;
    l = n->left;

    free_node(n);
    free_tree(r);
    free_tree(l);
}

NODE* insert_node(NODE *tree,void * new_item,int (*f)(void *,void*)){
    if(!tree)
        return create_node(new_item);
    if(f(new_item,tree->value)){
        tree->left = insert_node(tree->left,new_item,f);
    }
    else{
        tree->right = insert_node(tree->right,new_item,f);
    }
    return tree;
}

int save_tree(FILE* destiny,NODE *tree,int size){
    
    if(!tree)
        return 0;

    fwrite(tree->value,size,1,destiny);
    int l = save_tree(destiny,tree->left,size);
    int r = save_tree(destiny,tree->right,size);

    return l + r;
}

NODE * mount_tree(FILE* origin,int (*f)(void *,void*),int size){

    NODE * tree = NULL;
    while(!feof(origin)){
        char * buffer = malloc(sizeof(size));
        fread(buffer,size,1,origin);
        tree = insert_node(tree,buffer,f);
    }
    return tree;
}

/*TESTE*/
/*
int* init(int n){
    int *i = malloc(sizeof(int));
    *i = n;
    return i;
}
int comp(void *a,void *b){
    return *(int*)a < *(int*)b;
}

int main()
{

    NODE *tree = create_node(init(10));
    print_int_tree(tree,0);

    puts("etapa 1");

    insert_node(tree,init(4),comp);
    insert_node(tree,init(5),comp);
    print_int_tree(tree,0);

    puts("etapa 2");

    insert_node(tree,init(12),comp);
    insert_node(tree,init(3),comp);
    print_int_tree(tree,0);

    puts("etapa 3");

    FILE * f = fopen("escrita","w");

    save_tree(f,tree,sizeof(int));

    fclose(f);

    puts("etapa 4");

    free_tree(tree);

    puts("etapa 5");

    f = fopen("escrita","r");

    tree = mount_tree(f,comp,sizeof(int));

    print_int_tree(tree,0);

    fclose(f);

    return 0;
}
*/