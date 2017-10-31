#include "dropboxClient.h"

void print_commands() {
	puts("\n\nComandos disponíveis:");

	printf("\tupload ");
	cprintf(COLOR_GREEN, "<path/filename.ext>\n");

	printf("\tdownload ");
	cprintf(COLOR_GREEN, "<filename.ext>\n");

	puts("\tlist_server");
	puts("\tlist_client");
	puts("\tget_sync_dir");

	printf("\thelp ");
	cprintf(COLOR_GREEN, "command\n");

	puts("\tcredits");

	cprintf(COLOR_RED, "\texit");
}

void show_intro_message() {
	cprintf(COLOR_BLUE, "\tsyncBox\t");
	cprintf(COLOR_MAGENTA, INTRO_MESSAGE);

	printf("\nBem-vindo ao syncBox, ");
	cprintf(COLOR_GREEN, user.id);

	printf("\nPasta do usuário: ");
	cprintf(COLOR_GREEN, user.folder);
}

int is_not_exit_command(char* command) {
	return strcmp(command, COMMAND_EXIT) != 0;
}

int is_valid_command(char* command) {
	return
		strncmp(command, COMMAND_UPLOAD, strlen(COMMAND_UPLOAD)) == 0 ||
		strncmp(command, COMMAND_DOWNLOAD, strlen(COMMAND_DOWNLOAD)) == 0 ||
		strncmp(command, COMMAND_LS, strlen(COMMAND_LS)) == 0 ||
		strncmp(command, COMMAND_LC, strlen(COMMAND_LC)) == 0 ||
		strncmp(command, COMMAND_SYNC, strlen(COMMAND_SYNC)) == 0 ||
		strncmp(command, COMMAND_HELP, strlen(COMMAND_HELP)) == 0 ||
		strncmp(command, COMMAND_CREDITS, strlen(COMMAND_EXIT)) == 0 ||
		strncmp(command, COMMAND_EXIT, strlen(COMMAND_EXIT)) == 0;
}

void callCommand(char* command, char* attribute, int check) {
	if(strcmp(command, COMMAND_HELP) == 0) {
		if(check == COMMAND_WITH_ARGUMENTS) {
			command_help(attribute);
		} else {
			print_commands();
		}
	}
	else if(strcmp(command, COMMAND_UPLOAD) == 0) {
		if(check == COMMAND_WITH_ARGUMENTS) {
			command_upload(attribute);
		} else {
			command_help(COMMAND_UPLOAD);
		}
	}
	else if(strcmp(command, COMMAND_DOWNLOAD) == 0) {
		if(check == COMMAND_WITH_ARGUMENTS) {
			command_download(attribute);
		} else {
			command_help(COMMAND_DOWNLOAD);
		}
	} else if(strcmp(command, COMMAND_LS) == 0)
		command_listserver();
	else if(strcmp(command, COMMAND_LC) == 0)
		command_listclient();
	else if(strcmp(command, COMMAND_SYNC) == 0)
		command_getsyncdir();
	else if(strcmp(command, COMMAND_CREDITS) == 0)
		command_credits();
	else {
		if(check == COMMAND_WITH_ARGUMENTS) {
			command_help(attribute);
		} else {
			print_commands();
		}
	}
}

int parseCommand(char* command, char* commandName, char* commandAttrib) {
	unsigned int commandLength, offset;
	commandLength = strcspn(command, " ");

	strncpy(commandName, command, commandLength);
	commandName[commandLength] = '\0';

	int upload, download, help;
	upload 		= strncmp(command, COMMAND_UPLOAD, commandLength) 	== 0;
	download 	= strncmp(command, COMMAND_DOWNLOAD, commandLength) == 0;
	help 			= strncmp(command, COMMAND_HELP, commandLength) 		== 0;

	if(upload || download || help) {
		offset = sizeof(char) * commandLength + 1;

		if(offset >= strlen(command)) {
			if(help) {
				// Comando não necessita argumentos, de forma obrigatória.
				return COMMAND_WITH_NO_ARGUMENTS;
			}

			printf("\nNenhum argumento foi informado.\n");
			return NO_ARGUMENT_PROVIDED;
		} else {
			strncpy(commandAttrib, command + offset, strlen(command) - offset);

			commandAttrib[strlen(command) - offset] = '\0';
			return COMMAND_WITH_ARGUMENTS;
		}
	}

	return COMMAND_WITH_NO_ARGUMENTS;
}

void show_client_interface() {
	char comando_solicitado[11 + MAXPATH];
	char *valid;

	show_intro_message();
	print_commands();

	int not_exited = TRUE;
	while(not_exited) {
		printf("\nDigite seu comando: ");

		valid = fgets(comando_solicitado, sizeof(comando_solicitado)-1, stdin);
		if(valid != NULL) {
			comando_solicitado[strcspn(comando_solicitado, "\r\n")] = 0; //remover trailing \n
		}

		if(is_valid_command(comando_solicitado)) {
			not_exited = is_not_exit_command(comando_solicitado);
			if(not_exited) {
				char comando[20], atributo[100];

				int check = parseCommand(comando_solicitado, comando, atributo);
				callCommand(comando, atributo, check);
			}
		} else {
			printf("Comando '%s%s%s' inválido.\n", COLOR_GREEN, comando_solicitado, COLOR_RESET);
		}
	}

	close_connection();
}
