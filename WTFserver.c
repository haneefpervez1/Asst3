#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <openssl/sha.h>
#include <ctype.h>
#include<pthread.h>
struct fileNode {
	int nameLength;
	char* name;
	int contentLength;
	char* contents;
	struct fileNode * next;
};

char* hash(char*);
void create_server(int);
char *READ(int);
char* readFile (int);
char* sendFile(int , struct fileNode *);
void send_to_client(int , char * );
int addToList(struct fileNode**, int, char*, int, char*);
int printDir(char*, struct fileNode**);
int printDir_contents (char* directoryName, struct fileNode ** head);
int tokStringSendFiles(struct fileNode ** , char* );
int checkDir(char *);
void *get_in_addr(struct sockaddr *);
int remove_directory(const char *);
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) 
	{
        return &(((struct sockaddr_in*)sa)->sin_addr);
    	}
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char** argv) {
// -------------------------------------------------------> SOCKET CREATION
// ------------------------------------------------------->
// ------------------------------------------------------->
	if(argc!=2)
	{
	 printf("Error: Not enough Arguments\n");
	 return 1;
	}
	int sockfd, client_socket;
	struct addrinfo a, *b, *p;
	struct sockaddr_storage client;
	socklen_t size;
	int opt=1;
	int rv;
	char s[20];

    memset(&a, 0, sizeof(a));
    a.ai_family = AF_UNSPEC;
    a.ai_socktype = SOCK_STREAM;
    a.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, argv[1], &a, &b)) != 0) {
        printf("getadd Error");
        return 1;
    }
    // loop through all the results and bind to the first we can
    for(p = b; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            printf("server: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt,
                sizeof(int)) == -1) {
            printf("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            printf("server: bind");
            continue;
        }
        break;
    }
    	freeaddrinfo(b);
    	if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
   	}
    if (listen(sockfd, 5) == -1) {
        perror("listen");
        exit(1);
    }
     printf("server: waiting for connections...\n");
     
     while(1) {  // main accept() loop
        size = sizeof(client);
        client_socket = accept(sockfd,(struct sockaddr *)&client, &size);
        if (client_socket == -1) {
            perror("accept");
            continue;
        }
 inet_ntop(client.ss_family,get_in_addr((struct sockaddr *)&client), s, sizeof(s));
        printf("server: got connection from %s\n", s);
		char * buffer = READ(client_socket);
		if (strncmp("create", buffer, 6) == 0 ) {
			create_server(client_socket);
		}
		if(strncmp("checkout",buffer, 8)==0)
		{
			char * project = READ(client_socket);
			char path[7+strlen(project)];
			strcpy(path, "server");
			strcat(path, "/");
			strcat(path, project);
			DIR * dr = opendir(path);
			if (dr == NULL) {
			printf("error\n");
			return 1;
			}
			struct fileNode *fileList = NULL;
			int length = printDir_contents(path, &fileList);
			char* msg = sendFile(length, fileList);
			send_to_client(client_socket, msg);
		}
		if (strncmp("update", buffer, 6) == 0) {
			char * projName = READ(client_socket);
			char* path = malloc(strlen(projName) + strlen("server/") + strlen("/.Manifest") + 1);
			strcpy(path, "server/");
			strcat(path, projName);
			//int check = checkDir(path);
			strcat(path, "/.Manifest");
			int fd = open(path , O_RDONLY);
			if (fd < 0) {
				printf("Error: %s does not exist\n", projName);
				return 0;
			}
			char* string = readFile(fd);
			struct fileNode * head = NULL;
			int length = addToList(&head, strlen(path), path, strlen(string), string);
			char* serverMessage = sendFile(length, head);
			send_to_client(client_socket, serverMessage);
		}
		if (strncmp("upgrade", buffer, 7) == 0) {
			char* requestString = READ(client_socket);
			struct fileNode* fileList = NULL;
			int length = tokStringSendFiles(&fileList, requestString);
			char* messageToClient = sendFile(length, fileList);
			send_to_client(client_socket, messageToClient);
		}
		if (strncmp("destroy", buffer, 7) == 0) {
			char* projName = READ(client_socket);
			char* oldString = malloc(strlen("server/") + strlen(projName) + 1);
			strcpy(oldString, "server/");
			strcat(oldString, projName);
			if (checkDir(oldString) == 1) {
				printf("Error: %s does not exist\n", oldString);
			}
			remove_directory(oldString);
		}
		break;
	}
	close(client_socket);
	close(sockfd);
	return 0;
}
/*
	reads socket
*/
char * READ(int client_socket){
	char length[4];
	int x = read(client_socket, length, 4);
	int num = atoi(length);
	printf("Actually Read: %d\n", x);
	printf("Length Received: %d\n", num);
	char* buffer = malloc(num+1);
	//char * temp = buffer;
	read(client_socket, buffer, num+1);
	printf("%s\n", buffer);
	return buffer;
}
/*
	generates hash
*/
char* hash (char * contents) {
	int x;
	unsigned char hash [SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, contents, strlen(contents));
	SHA256_Final(hash, &sha256);
	char* string = malloc(SHA256_DIGEST_LENGTH*2 + 1);
	for (x = 0; x < SHA256_DIGEST_LENGTH; x++) {
		sprintf(string+(x*2), "%02x", hash[x]);
	}
	int index = SHA256_DIGEST_LENGTH * 2;
	string[index] = '\0';
	return string;
}
char* readFile (int fd) {
	int i=1;
	char buff;
	int x = read(fd, &buff, 1);
	char string[10000];
	string[0]=buff; 
	while((x != 0) || (buff!='\n'))
	{
		x = read(fd, &buff, 1);
			string[i]=buff;
		i++;
	}
	int index=i;
	char stringPtr[index];
	for(i=0;i<index;i++)
	{
	 stringPtr[i]=string[i];
	}
	char * l = stringPtr;
	return l;
}
/*
	creates project
*/
void create_server(int client_socket)
{
		char* project = READ(client_socket);
		struct stat st = {0};
		if(stat(project, &st) ==-1)
		{
		 mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		 char server[1000]= "server/";
		 char  directory[1000];
		 strcpy(directory, server);
		 strcat(directory, project);
		 if (checkDir(directory) == 0) {
			printf("Error: %s already exists\n", directory);
			return;
		 }
		 mkdir(directory, 0700);
		 char manifest[1000];
		 strcpy(manifest, directory);
		 strcat(manifest, "/");
		 char * path = strcat(manifest, ".Manifest");
		 open(path, O_RDWR | O_CREAT, mode);
		 send_to_client(client_socket, "created");
		}
		else
		{
		write(client_socket, "failed", sizeof("failed"));
		}
}
/*
	The message sent is of the format
	s:<# of files>:<length of filename>:<filename>:<length of contents>:<contents>:<next file following same format>
*/
char* sendFile(int listLength, struct fileNode* head) {	
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
	char* serverMessage = malloc(length);
	strcpy(serverMessage, "s:");
	strcat(serverMessage, listLen); // Should be > 0
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
	return serverMessage;
}
/*
	writes to client
*/
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
/*
	adds to file ll
*/
int addToList(struct fileNode ** head, int nameLength, char* name, int contentLength, char* contents) {
	int counter = 1;
	struct fileNode* temp = (struct fileNode*)malloc(sizeof(struct fileNode));
	temp->nameLength = nameLength;
	temp->name = malloc(nameLength+1);
	strcpy(temp->name, name+7);
	temp->name[nameLength] = '\0';
	temp->contentLength = contentLength;
	temp->contents = malloc(contentLength+1);
	strcpy(temp->contents, contents);
	temp->contents[contentLength] = '\0';
	if (*head == NULL) {
		*head = temp;
	} else {
		struct fileNode * ptr = *head;
		while (ptr->next != NULL) {
			ptr = ptr->next;
			counter++;
		}
		counter++;
		ptr->next = temp;
	}
	return counter;
}
/*
	prints directory
*/
int printDir (char* directoryName, struct fileNode ** head) {
	DIR *d;
	struct dirent *dir;
	d = opendir(directoryName);
	int dirNameLen = strlen(directoryName);
	int fileListLen = 0;
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			char* filename = malloc(dirNameLen + strlen(dir->d_name) + 2);
			strcpy(filename, directoryName);
			strcat(filename, "/");
			strcat(filename, dir->d_name);
			printf("%s\n", filename);
			if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
				int fd = open(filename, O_RDONLY);
				if (fd < 0) {
					printf("Error: %s does not exist\n", filename);
				}
				char* contents = readFile(fd);
				close(fd);
				char* hashString = hash(contents);
				fileListLen = addToList(head, strlen(filename), filename, strlen(hashString), hashString);
			}
		}
		closedir(d);
	}
	return fileListLen;
}

int printDir_contents (char* directoryName, struct fileNode ** head) {
	DIR *d;
	struct dirent *dir;
	d = opendir(directoryName);
	int dirNameLen = strlen(directoryName);
	int fileListLen = 0;
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			char* filename = malloc(dirNameLen + strlen(dir->d_name) + 2);
			strcpy(filename, directoryName);
			strcat(filename, "/");
			strcat(filename, dir->d_name);
			printf("%s\n", filename);
			if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
				int fd = open(filename, O_RDONLY);
				if (fd < 0) {
					printf("Error: %s does not exist\n", filename);
				}
				char* contents = readFile(fd);
				close(fd);
				//char* hashString = hash(contents);
				fileListLen = addToList(head, strlen(filename), filename, strlen(contents), contents);
			}
		}
		closedir(d);
	}
	return fileListLen;
}
/*
	takes string, tokenizes it, sends relevent files
*/
int tokStringSendFiles(struct fileNode ** head, char* clientString) {
	int fileListLen = 0;
	char* token = strtok(clientString, ":");
	token = strtok(NULL, ":");
	while (token != NULL) {
		token = strtok(NULL, ":");
		if (token != NULL) {
			int LENGTH = strlen(token) + strlen("server/");
			char * path = malloc(LENGTH + 1);
			strcpy(path, "server/");
			strcat(path, token);
			path[LENGTH] = '\0';
			int fd = open(path, O_RDONLY);
			if (fd < 0) {
					printf("Error: %s does not exist\n", path);
					return 0;
				}
			char* contents = readFile(fd);
			int length = strlen(contents);
			fileListLen = addToList(head, strlen(path), path, length, contents);
		}
	}
	return fileListLen;
}
/*
	deletes directories
*/
int remove_directory(const char *path) {
	DIR *d = opendir(path);
	size_t path_len = strlen(path);
	int r = -1;
    if (d) {
		struct dirent *p;
      	r = 0;
      	while (!r && (p=readdir(d))) {
          	int r2 = -1;
          	char *buf;
          	size_t len;
          	if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
             	continue;
          	}
		  	len = path_len + strlen(p->d_name) + 2; 
          	buf = malloc(len);
          	if (buf) {
            	struct stat statbuf;
             	snprintf(buf, len, "%s/%s", path, p->d_name);
             	if (!stat(buf, &statbuf)) {
                if (S_ISDIR(statbuf.st_mode)) {
                	r2 = remove_directory(buf);
                }
                else {
                	r2 = unlink(buf);
                }
             }
             free(buf);
          }
		  r = r2;
      }
      closedir(d);
   }
   if (!r) {
      r = rmdir(path);
   }
   return r;
}
int checkDir(char * path)
{
 	struct stat st = {0};
	if (stat(path, &st) != -1)
	{
    	printf("Error:Project name already exists\n");
    	return 0;
	}
  return 1;
}
