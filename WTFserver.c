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

int main(int argc, char** argv) {
	char server_message[256] = "You have reached thnaLCNlle server";
	//for (;;) {	
		int server_socket;
		server_socket = socket(AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in server_address;
		server_address.sin_family = AF_INET;
		server_address.sin_port = htons(9002);
		server_address.sin_addr.s_addr = INADDR_ANY;
		bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));
		listen(server_socket, 5);
		int client_socket;
		client_socket = accept(server_socket, NULL, NULL);
		if (client_socket > 0) {
			printf("connection acceptance test success\n");
		} else {
			printf("connection acceptance failure\n");
		}
		char server_response[256];
		char server_Response[256];
		write(client_socket, server_message, sizeof(server_message));
		read(client_socket, &server_response, sizeof(server_response));
		read(client_socket, &server_Response, sizeof(server_Response));
		mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		//int length = sizeof(server_response) + 7;
		char clientFile[500] = "server/"; 
		char* path = strcat(clientFile, server_response);
		int fd = open(path, O_RDWR | O_CREAT, mode);
 		//write(clientFile, "a", strlen("a"));
		printf("%s %d %s", server_response, fd, server_Response);
//	}
	close(server_socket);
	return 0;
}

