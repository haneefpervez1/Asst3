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

struct fileNode {
	int nameLength;
	char* name;
	int contentLength;
	char* contents;
	struct fileNode * next;
};

void hash(char*);
void create_server(int);
char *READ(int);
char* readFile (int);
char* sendFile(int , struct fileNode *);
void send_to_client(int , char * );
int addToList(struct fileNode**, int, char*, int, char*);

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
		int opt = 1;
		if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
			perror("setsocketopt");
			exit(1);
		}
		if(bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address))) {
			perror("bind failed");
			exit(1);
		}
		if(listen(server_socket, 5)) {
			perror("listen failed");
			exit(1);
		}
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
		if(strncmp("checkout",buffer, 8)==0)
		{
			char * project = READ(client_socket);
			char path[7+strlen(project)];
			strcpy(path, "server");
			strcat(path, "/");
			strcat(path, project);
			printf("%s\n", path);
			struct dirent *de;
			DIR * dr = opendir(path);
			if (dr == NULL) {
			printf("error\n");
			return 1;
			}
			while ((de = readdir(dr)) != NULL) {
				//printf("%s\n", de->d_name);
				if(de->d_type == DT_REG)
				{
				char Path[strlen(path)+strlen(de->d_name)+1];
				strcpy(Path, path);
				strcat(Path, "/"); 
				strcat(Path, de->d_name);
				struct fileNode *fileList = NULL;
				int temp = open(Path, O_RDONLY);
				char * tempS =readFile(temp);
				int size = strlen(tempS);
				printf("Content Length: %d\n", size);
				printf("Contents of File: %s\n", tempS);
				int l = addToList(&fileList, strlen(Path), Path, strlen(tempS), tempS);
				send_to_client(client_socket, (sendFile(l, fileList)));
				}
			}
			//printf("%s", temp);
		}
		if (strncmp("update", buffer, 6) == 0) {
			//write(client_socket, "update command received", strlen("update command received"));
			char * projName = READ(client_socket);
			printf("projName: %s\n", projName);
			char* path = malloc(strlen(projName) + strlen("server/") + strlen("/manifest.txt") + 1);
			strcpy(path, "server/");
			strcat(path, projName);
			strcat(path, "/manifest.txt");
			printf("path: %s\n", path);
			struct fileNode* fileList= NULL;
			int length = addToList(&fileList, strlen("manifest.txt"), "manifest.txt", strlen("a"),"a");
			length = addToList(&fileList, strlen("abcde"), "abcde", strlen("abc"), "abc");
			printf("fileList length is %d\n", length);
			char* manifestMessage = sendFile(length, fileList);
			send_to_client(client_socket, manifestMessage);
		}
		//hash("systems");
	close(server_socket);
	return 0;
}
char * READ(int client_socket){
	//char length[4];
	/*int length = 0;
	read(client_socket, &length, sizeof(length));
	//int num = atoi(length);
	//length = length -47;
	printf("Length Received: %d\n", length);
	//printf("%s\n", length);
	char *buffer = malloc(length+1);
	read(client_socket, buffer, length+1);
	buffer[length] = '\0';*/
	char length[4];
	read(client_socket, length, 4);
	int num = atoi(length);
	printf("%s\n", length);
	printf("Length Received: %d\n", num);
	//char *buffer = malloc(sizeof(chcar) * num);
	char buffer[num];
	char * temp = buffer;
	read(client_socket, buffer, num+1);
	printf("%s\n", buffer);
	return temp;
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
	//printf("%c\n", buff[0]);
	char string[10000];
	string[0]=buff[0]; 
	char* stringPtr = string;
	while(x != 0)
	{
		x = read(fd, buff, 1);
		if(buff[0]!='\n')
		{
			//printf("%c\n", string[i]);
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
/*
	The message sent is of the format
	s:<# of files>:<length of filename>:<filename>:<length of contents>:<contents>:<next file following same format>
*/
char* sendFile(int listLength, struct fileNode* head) {	
	/*
	int fd = open(filename, O_RDONLY);
	char* string = readFile(fd);
	printf("string: %s\n", string);
	char* message = malloc(strlen("s:manifest:") +strlen(string) + 1);
	strcpy(message, "s:manifest:");
	strcat(message, string);
	printf("message: %s\n", message);
	return message;
	*/
	int length = 0;
	struct fileNode *ptr = head;
	while (ptr != NULL) {
		char buffer1[10];
		char buffer2[10];
		sprintf(buffer1, "%d", ptr->nameLength);
		sprintf(buffer2, "%d", ptr->contentLength);
		length += strlen(buffer1) + strlen(buffer2) + strlen(ptr->name) + strlen(ptr->contents) + 4;
		ptr = ptr->next;
	}
	char listLen [10];
	sprintf(listLen, "%d", listLength);
	length += strlen(listLen) + 3;
	printf("sendFile length %d\n", length);
	char* serverMessage = malloc(length);
	strcpy(serverMessage, "s:");
	strcat(serverMessage, "1"); // Should be > 0
	ptr = head;
	while (ptr != NULL) {
		char buffer1[10];
		char buffer2[10];
		sprintf(buffer1, "%d", ptr->nameLength);
		sprintf(buffer2, "%d", ptr->contentLength);
		strcat(serverMessage, ":");
		strcat(serverMessage, buffer1);
		strcat(serverMessage, ":");
		strcat(serverMessage, ptr->name);
		strcat(serverMessage, ":");
		strcat(serverMessage, buffer2);
		strcat(serverMessage, ":");
		strcat(serverMessage, ptr->contents);
		ptr = ptr->next;
	}
	printf("serverMessage: %s\n", serverMessage);
	return serverMessage;
}
void send_to_client(int network_socket, char * string)
{
 int len = strlen(string);
 printf("String Name: %s\n", string);
 char c[4];
 sprintf(c, "%d", len);
 printf("Length of String: %s\n", c);
 //int number = htonl(len);
 write(network_socket, c, 4);
 write(network_socket, string, (strlen(string)+1));
}
int addToList(struct fileNode ** head, int nameLength, char* name, int contentLength, char* contents) {
	int counter = 0;
	struct fileNode* temp = (struct fileNode*)malloc(sizeof(struct fileNode));
	temp->nameLength = nameLength;
	temp->name = malloc(nameLength+1);
	strcpy(temp->name, name);
	temp->name[nameLength] = '\0';
	temp->contentLength = contentLength;
	temp->contents = malloc(contentLength+1);
	strcpy(temp->contents, contents);
	temp->contents[contentLength] = '\0';
	//printf("a structnode containing %d %s %d %s will be added\n", temp->nameLength, temp->name, temp->contentLength, temp->contents); 
	if (*head == NULL) {
		*head = temp;
		counter = 1;
	} else {
		struct fileNode * ptr = *head;
		while (ptr->next != NULL) {
			ptr = ptr->next;
			counter++;
		}
		ptr->next = temp;
	}
	printf("a structnode containing %d %s %d %s has been added\n",temp->nameLength, name, contentLength, contents); 
	return counter;
}

