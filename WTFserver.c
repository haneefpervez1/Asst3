#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

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
		send(client_socket, server_message, sizeof(server_message), 0);
		recv(client_socket, &server_response, sizeof(server_response), 0);
		printf("%s", server_response);
//	}
	//close(server_socket);
	return 0;
}

