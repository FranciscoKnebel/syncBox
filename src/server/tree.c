#include "tree.h"

NODE* create_node(void *value){
/*
criação de um nodo com o valor passado, void para ser genérico
*/
    NODE* new_node = (NODE *)malloc(sizeof(NODE));
    new_node->value = value;
    new_node->left=NULL;
    new_node->right=NULL;
    return new_node;
}

void print_int_tree(NODE* n,int depth){

/*
print de uma árvore caso ela seja inteira!
Útil para debug da árvore
*/
    if(!n)
        return;
    int i=0;
    for(;i<depth;i++)
        printf("-");
    printf("%p\n",(int*)n->value);
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

    /*
    libera recursivamente a árvore, note que é dado free em value
    logo valores que são com ponteiros terão suas referências 
    perdidas!
    */
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
    /*
    principal função da árvore, com auxilio de uma função f do usuário
    insere o valor na árvore!Note que f deve lidar com o valor passado de inserção
    e dos nós!
    */
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
#include "dropboxServer.h"
int save_tree(FILE* destiny,NODE *tree,int size){
    /*
    salva a árvore em um arquivo, NÃO CHECA se houve sucesso
    */
    if(!tree)
        return 0;

    if(!destiny)
        return 1;

    int bytes_escritos = fwrite(tree->value,1,size,destiny);
    if( size != bytes_escritos)
        return 1;

    int l = save_tree(destiny,tree->left,size);
    int r = save_tree(destiny,tree->right,size);

    return (l + r)%2;
}

NODE * mount_tree(FILE* origin,int (*f)(void *,void*),int size,int n_nodes){
    /*
    leitura de arquivo para obter a árvore salva, com alocação!
    */
    NODE * tree = NULL;
    while(n_nodes>0){
        char * buffer = malloc(size);
        if(size != fread(buffer,1,size,origin)){
            fprintf(stderr, "leitura não possível de %d bytes",size);
            exit(1);
        }
        tree = insert_node(tree,(void*)buffer,f);
        n_nodes--;
    }
    return tree;
}

void* get_value(NODE *tree, void *target, int (*f)(void *,void*)){
    /*
    consulta de um valor na árvore, deve haver uma função que retorna verdade quando encontrado!
    */


    if(!tree)
        return NULL;

    if(f(tree->value,target))
        return tree->value;

    void * l = get_value(tree->left,target,f);
    if(l != NULL)
        return l;
    void * r = get_value(tree->right,target,f);
    if(r != NULL)
        return l;

    return NULL;
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

int find(void *a,void *b){
    return *(int*)a == *(int*)b;
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

    puts("etapa 6");

    int target = 7;
    char *resultado;
    do{
        void * res_busca = get_value(tree,(void*)init(target),find);
        resultado = (res_busca == NULL) ? strdup("não encontrado\n") : strdup("encontrado");
        printf("procurando %d - resultado %s\n",target,resultado);
    }while(target--);

    return 0;
}
*/