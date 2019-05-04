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
void create_server(int);
char *READ(int);
char* readFile (int);

int main(int argc, char** argv) {
	char server_message[256] = "You have reached the server";
	// -------------------------------------------------------> SOCKET CREATION
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
		write(client_socket, server_message, sizeof(server_message));
	// ------------------------------------------------------------------------>
		char * buffer = READ(client_socket);
		//printf("%s\n", buffer);
		if(strncmp("checkout",buffer, 8)==0)
		{
			READ(client_socket);
			//printf("%s", temp);
		}
		if (strcmp("update", buffer) == 0) {
			char* projName = READ(client_socket);
			printf("projName: %s\n", projName);
		}
		//hash("systems");
	//close(server_socket);
	return 0;
}
char * READ(int client_socket){
	char length[4];
	read(client_socket, length, sizeof(length));
	int num = atoi(length);
	printf("Length Received: %d\n", num);
	//printf("%s\n", length);
	char *buffer = malloc(num+1);
	read(client_socket, buffer, num+1);
	buffer[num] = '\0';
	printf("%s\n", buffer);
	return buffer;
}

void hash (char * contents) {
	int x;
	unsigned char hash [SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, contents, strlen(contents));
	SHA256_Final(hash, &sha256);
	for (x = 0; x < SHA256_DIGEST_LENGTH; x++) {
		printf("%02x", hash[x]);
	}
}
char* readFile (int fd) {
	int i=1;
	char buff[1];
	int x = read(fd, buff, 1);
	char string[10000];
	string[0]=buff[0]; 
	char* stringPtr = string;
	while(x != 0)
	{
		x = read(fd, buff, 1);
		if(buff[0]!='\n')
		{
			string[i]=buff[0];
		} 
		i++;
	}
	
	return stringPtr;
}
void create_server(int client_socket)
{
		char project[256];
		read(client_socket, project, sizeof(project));
		struct stat st = {0};
		if(stat(project, &st) ==-1)
		{
		 mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
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
}
