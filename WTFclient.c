#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

struct manifest{
	char * path;
	int version;
	int hash;
	struct manifest * next;
};
int configure(char*, char*);
int create (char *);
int main (int args, char** argv) {
	
	int network_socket;
	network_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(9002);
	server_address.sin_addr.s_addr = INADDR_ANY;
	
	int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
	if (connection_status != 0) {
		printf("Error with connection\n");
	}
	char server_response[256];
	char client_message[256] = "test.txt\n";
	read(network_socket, &server_response, sizeof(server_response));
	printf("The server sent the data: %s\n", server_response);
	write(network_socket, &client_message, sizeof(client_message));

	if (strcmp(argv[1], "configure") == 0) {
		char configureMessage[20] = "configure:2:tst.txt";
		write(network_socket, &configureMessage, sizeof(configureMessage));
		configure(argv[2], argv[3]);
	}
	else if (strcmp(argv[1], "create")==0)
	{
	 char * createmsg = malloc(sizeof(argv[2])+1);
	 strcpy(createmsg, argv[2]);
	 write(network_socket, &createmsg, sizeof(createmsg));
	 create(argv[2]);
	}
	close(network_socket);
	return 0;
}

int configure(char* hostname, char* port) {
	printf("hostname: %s port: %s \n", hostname, port);
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
 	int configure_file = creat("configure.txt", mode);
	write(configure_file, hostname, sizeof(hostname));
	write(configure_file, " ", strlen(" "));
	write(configure_file, port, sizeof(port));
	return 1;
}
int create (char * projectname)
{
 return 1;
}
