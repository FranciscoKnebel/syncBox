#include "dropboxClient.h"

void print_commands() {
	puts("\nComandos do cliente:");

	printf("\tupload ");
	cprintf(ANSI_COLOR_GREEN, "<path/filename.ext>\n");

	printf("\tdownload ");
	cprintf(ANSI_COLOR_GREEN, "<filename.ext>\n");

	puts("\tlist_server");
	puts("\tlist_client");
	puts("\tget_sync_dir");

	printf("\thelp ");
	cprintf(ANSI_COLOR_GREEN, "command\n");

	cprintf(ANSI_COLOR_RED, "\texit");
}

void show_intro_message() {
	cprintf(ANSI_COLOR_BLUE, "\tsyncBox\t");
	cprintf(ANSI_COLOR_MAGENTA, "v0.0.1 - 03/11/2017\n");
	printf("\nGabriel Pittol\nFrancisco Knebel\nLuciano Zancan\nMatheus Krause\n");
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
	else {
		if(check == COMMAND_WITH_ARGUMENTS) {
			command_help(attribute);
		} else {
			print_commands();
		}
	}
}

int parseCommand(char* command, char* commandName, char* commandAttrib) {
	int commandLength, offset;

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
	char comando_solicitado[100], nome_arquivo[100];

	show_intro_message();
	print_commands();

	int not_exited = is_not_exit_command(comando_solicitado);
	while(not_exited) {
		printf("\nDigite seu comando: ");

		fgets(comando_solicitado, sizeof(comando_solicitado), stdin);
		comando_solicitado[strcspn(comando_solicitado, "\r\n")] = 0; //remover trailing \n

		if(is_valid_command(comando_solicitado)) {
			not_exited = is_not_exit_command(comando_solicitado);
			if(not_exited) {
				char comando[20];
				char atributo[100];

				int check = parseCommand(comando_solicitado, comando, atributo);
				callCommand(comando, atributo, check);
			}
		} else {
			printf("Comando '%s%s%s' inválido.\n", ANSI_COLOR_GREEN, comando_solicitado, ANSI_COLOR_RESET);
		}
	}
}
