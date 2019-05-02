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
#include <openssl/sha.h>

void hash(char*);

int main(int argc, char** argv) {
	char server_message[256] = "You have reached the server";
	//for (;;) {	
		int portNum = atoi(argv[1]);
		int server_socket;
		server_socket = socket(AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in server_address;
		server_address.sin_family = AF_INET;
		server_address.sin_port = htons(portNum);
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
		char project[256];
		//char server[6];
		write(client_socket, server_message, sizeof(server_message));
		read(client_socket, &server_response, sizeof(server_response));
		read(client_socket, project, sizeof(project));
		mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		//int length = sizeof(server_response) + 7;
		char clientFile[1000] = "server/"; 
		char* path = strcat(clientFile, server_response);
		int fd = open(path, O_RDWR | O_CREAT, mode);
 		//write(clientFile, "a", strlen("a"));
		printf("%s %d %s\n", server_response, fd, project);
		struct stat st = {0};
		if(stat(project, &st) ==-1)
		{
		 char server[1000]= "server/";
		 char  directory[1000];
		 strcpy(directory, server);
		 //printf("%s\n", directory);
		 printf("%s\n", server);
		 strcat(directory, project);
		 printf("%s\n", project);
		 //printf("%s", directory);
		 mkdir(directory, 0700);
		 char manifest[1000];
		 strcpy(manifest, directory);
		 strcat(manifest, "/");
		 char * path = strcat(manifest, "manifest.txt");
		 open(path, O_RDWR | O_CREAT, mode);
		 write(client_socket, "created", sizeof("created"));
		}
		else
		{
		write(client_socket, "failed", sizeof("failed"));
		}
		//read(client_socket, &server, sizeof(server)); 
		//printf("%s", server_response); 
		hash("systems");
	//}
	//close(server_socket);
	return 0;
}

void hash (char * contents) {
<<<<<<< HEAD
	int x;
	unsigned char hash [SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, contents, strlen(contents));
	SHA256_Final(hash, &sha256);
	for (x = 0; x < SHA256_DIGEST_LENGTH; x++) {
		printf("%02x", hash[x]);
	}
=======
>>>>>>> hs770-branch
}

