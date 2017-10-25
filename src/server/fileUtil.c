#include "fileUtil.h"

int read_file(char **content,char *file_name){

	if(!file_name)
		return EXIT_FAILURE;

	struct stat sb;

	stat(file_name,&sb);

	*content = malloc(sizeof(sb.st_size));

	FILE* f = fopen(file_name,"r");
	if(!f)
		return EXIT_FAILURE;

	fread(*content,sb.st_size,1,f);

	fclose(f);

	return EXIT_SUCCESS;	
}

int remove_file(char *file_name){

	if(!file_name)
		return EXIT_FAILURE;

	if(remove(file_name) !=0)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

int write_file(char *content,int size,char *file_name){


	if(!content | !file_name)
		return EXIT_FAILURE;

	FILE *f = fopen(file_name,"w");

	if(!f)
		return EXIT_FAILURE;

	fwrite(content,size,1,f);
	fclose(f);

	return EXIT_SUCCESS;

}

int get_file_extension(char *file_name,char **file_extension){

	if(!file_name)
		return EXIT_FAILURE;

	int size = strlen(file_name);

	char *pc = strrchr(file_name,'.');
	int location = pc-file_name+1;
	if(!pc)
		return EXIT_FAILURE;

	*file_extension = malloc(sizeof(size-location+1));
	strncpy(*file_extension,file_name+location,size-location+1);
	return EXIT_SUCCESS;
}

int get_last_modification_time(char *file_name,char **last_modfication){

	struct stat sb;

	if(!file_name || stat(file_name,&sb)!= 0)
		return EXIT_FAILURE;

	*last_modfication = strdup(ctime(&sb.st_mtime));

	return EXIT_SUCCESS;
}
/*
int main(int argc,char *argv[]){

	struct stat sb;

	if(argc <2)
		exit(EXIT_FAILURE);

	puts("teste 1-status");
	stat(argv[1],&sb);

	printf("File type:                ");

	switch (sb.st_mode & S_IFMT) {
		case S_IFBLK:  printf("block device\n");            break;
		case S_IFCHR:  printf("character device\n");        break;
		case S_IFDIR:  printf("directory\n");               break;
		case S_IFIFO:  printf("FIFO/pipe\n");               break;
		case S_IFLNK:  printf("symlink\n");                 break;
		case S_IFREG:  printf("regular file\n");            break;
		case S_IFSOCK: printf("socket\n");                  break;
		default:       printf("unknown?\n");                break;
	}

	printf("File size:%lld bytes\n",(long long) sb.st_size);

	printf("Last file access:         %s", ctime(&sb.st_atime));
	printf("Last file modification:   %s", ctime(&sb.st_mtime));

	puts("teste 2-tempo de modificação");
	char *s;
	assert( get_last_modification_time(argv[1],&s) == EXIT_SUCCESS);
	puts(s);

	puts("teste 3-extenção");
	free(s);
	assert(get_file_extension(argv[1],&s)== EXIT_SUCCESS);
	puts(s);

	puts("teste 4-escrever arquivo");
	assert(write_file(s,strlen(s),"teste.txt") == EXIT_SUCCESS);

	free(s);
	puts("teste 5-ler arquivo");
	assert(read_file(&s,"teste.txt")==EXIT_SUCCESS);
	puts(s);

	puts("teste 6-remover arquivo");
	assert(remove_file("teste.txt")==EXIT_SUCCESS);

	exit(EXIT_SUCCESS);
}
*/
