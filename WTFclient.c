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
#include <openssl/sha.h>
#include <arpa/inet.h>
#include <ctype.h>

struct manifestNode{
	char * path;
	char* version;
	char* hash;
	struct manifestNode * next;
};

int configure(char*, char*);
int getPortNum();
void addFile(char*, char*);
void read_string(char *);
//int is_digit(char *);
char* hash (char *);
void send_to_server(int, char *);
char * READ(int);
void compareManifests(char* , char* );
void addManifestList(struct manifestNode ** , char* , char* , char* );
char* readLine(int);
char* readFile (int );
void checkUpLoad(struct manifestNode * , struct manifestNode * );
int checkIfPresent (struct manifestNode* , struct manifestNode*, char* );
int getClientFilePath(char* );
void checkModify(struct manifestNode * , struct manifestNode * );
void checkAdd(struct manifestNode * , struct manifestNode * );
void checkDelete(struct manifestNode * , struct manifestNode * );

int main (int args, char** argv) {
	char client [100] = "client/";
	//char * command;
	if (strcmp(argv[1], "configure") == 0) {
		//char configureMessage[20] = "configure:2:tst.txt";
		//write(network_socket, &configureMessage, sizeof(configureMessage));
		printf("configure %d\n", configure(argv[2], argv[3]));
	}
// ----------------------------------------------------------------------------------> SOCKET CREATION
	int network_socket;
	network_socket = socket(AF_INET, SOCK_STREAM, 0);
	int port = getPortNum();
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = INADDR_ANY;
	
	int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
	if (connection_status != 0) {
		printf("Error with connection\n");
	}
	char server_response[256];
	read(network_socket, &server_response, sizeof(server_response));
	printf("The server sent the data: %s\n", server_response);
// ------------------------------------------------------------------------------------------------------------------> SOCKET CREATION
	if (strcmp(argv[1], "checkout") == 0){
		//command = argv[1];
		//strcat(command, argv[2]);
		struct stat st = {0};

		if (stat("/some/directory", &st) != -1) {
    		 printf("Project name already exists");
		}
		else
		{
		send_to_server(network_socket, argv[1]);
		send_to_server(network_socket, argv[2]);
		char* string = READ(network_socket);
		printf("contents of manifest: %s\n", string);
		read_string(string);
		//printf("%s", argv[1]);
		}
	}
	
	if (strcmp(argv[1], "create")==0)
	{
	 char * createmsg = malloc(sizeof(argv[2])+1);
	 char direct[100];
	 char choice [100];
	 char path [100];
	 strcpy(createmsg, argv[2]);
	 printf("%s", createmsg);
	 write(network_socket, createmsg, sizeof(createmsg));
	 read(network_socket, choice, sizeof(choice));
	 if(choice[0]=='c')
	 {
	  strcpy(direct, client);
	  strcat(direct, createmsg);
	  mkdir(direct, 0700);
	  strcpy(path, direct);
	  strcat(path, "/manifest.txt");
	  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	  open(path,O_RDWR | O_CREAT, mode);
	 }
	}
	if (strcmp(argv[1], "add") == 0) {
		addFile(argv[2], argv[3]);
	}
	if (strcmp(argv[1], "update") == 0) {
		send_to_server(network_socket, argv[1]);
		send_to_server(network_socket, argv[2]);
		char* string = READ(network_socket);
		//printf("contents of manifest: %s\n", string);
		compareManifests(string, argv[2]);
		//read_string(string);
		//printf("manifest %s\n", manifestString);
		//char updateMessage[256];
		//read(network_socket, &updateMessage, sizeof(updateMessage));
		/*printf("update message: %s\n", updateMessage);
		if (strcmp(updateMessage, "update command received") == 0) {
			send_to_server(network_socket, argv[2]);
		}
		char* manifestString = READ(network_socket);
		printf("manifest %s\n", manifestString);*/
	}
	close(network_socket);
	return 0;
}
void send_to_server(int network_socket, char * string)
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
int configure(char* hostname, char* port) {
	if (access(".configure", F_OK) != -1) {
		printf("configure file already exists\n");
		return 1;
	} 
	printf("hostname: %s port: %s \n", hostname, port);
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
 	int configure_file = creat(".configure", mode);
	write(configure_file, port, strlen(port));
	write(configure_file, " ", strlen(" "));
	write(configure_file, hostname, strlen(hostname));
	return -1;
}
int getPortNum() {
	if (access(".configure", F_OK) != -1) {
		int fd = open(".configure", O_RDONLY);
		int i=1;
		char buff[1];
		int x = read(fd, buff, 1);
		char string[10000];
		string[0]=buff[0]; 
		 	while(x != 0)
		 	{
		 	x = read(fd, buff, 1);
				if (buff[0] == ' ') {
					break;
				}
			 	 if(buff[0]!='\n')
			 	 {
			 	 	string[i]=buff[0];
			 	 } 
		 	 i++;
		 	}
		int port = atoi(string);
		//printf("port %d\n", port);
		return port;
	}
	return -1;
}

char* readFile (int fd) {
	int i=1;
	char buff[1];
	int x = read(fd, buff, 1);
	char* string = malloc(10000);
	string[0]=buff[0]; 
	while(x != 0)
	{
		x = read(fd, buff, 1);
		//if(buff[0]!='\n')
	//	{
			string[i]=buff[0];
		//} 
		i++;
	}
	
	return string;
}
char* readLine(int fd) {
	int i = 1;
	char* buff = malloc(1);
	int x = read(fd,buff,1);
	char* string = malloc(10000);
	string[0] = buff[0];
	//char* stringPtr = string;
	while (buff[0] != '\n' && x != 0) {
		x = read(fd, buff, 1);
		string[i] = buff[0];
		i++;
	}
	string[i-1] = '\0';
	return string; 
}
/*int is_digit(char * string)
{
 int i;
 for(i=0;i<=strlen(string);i++)
 {
  if(isdigit(string[i])==0)
  {
  	return 0;
  }
 }
 return 1;
}*/

void read_string(char * string)
{
	printf("Reading.\n");
	printf("Reading..\n");
	printf("Reading...\n");
 	char * token = strtok(string, ":");
 	token = strtok(NULL,":");
 	int fileNum = atoi(token);
 	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
 	while(token!=NULL)
 	{
 	 token = strtok(NULL,":");
 	 	if(fileNum!=0)			//Create a file
 	 	{
		 token = strtok(NULL, ":");	// Project Name
		 char* filename = malloc(strlen(token) + strlen("client/")+1);
		 strcpy(filename, "client/");
		 strcat(filename, token);
	 	 int fd = open(filename,O_RDWR | O_CREAT, mode);
		 printf("this is the fd %d\n", fd); 
	 	 printf("File %s Created:\n", filename);
	 	 token = strtok(NULL, ":");	// Bytes to write
	 	 int bytes = atoi(token);
	 	 token = strtok(NULL, ":");	// What to Write
	 	 write(fd, token, bytes);
	 	 printf("Content %s written:\n", token);
	 	 close(fd);
	 	 fileNum--;
		}
 	}
}
void addFile (char* projName, char* filename) {				// still need to deal with if project doesnt exist and if file is already in manifest
	
	int length = strlen(projName) + strlen(filename) + strlen("client/");
	printf("length %d\n", length);
	char* path = malloc(length+2);
	strcpy(path, "client/");
	strcat(path, projName);
	strcat(path, "/");
	strcat(path, filename);
	int fd = open(path, O_RDONLY);
	char* string = readFile(fd);
	printf("string: %s\n", string);
	char* hashString = hash(string);
	printf("new file: %s fd: %d\nThese are the contents: %s\n", path, fd, hashString);

	char* manifestPath = malloc(strlen(projName) + 14 +strlen("client/"));
	strcpy(manifestPath, "client/");
	strcat(manifestPath, projName);
	strcat(manifestPath, "/manifest.txt");
	printf("manifest file %s\n", manifestPath);
	
	int fileD = open(manifestPath, O_RDWR);
	printf("fileD %d\n", fileD);
	
	char buff[1];
	int x = read(fileD, buff, 1);
	char* firstLine = malloc(20);
	if (x == 0 ) {
		write(fileD, "$0\n", strlen("$0\n"));
		printf("manifest is empty\n");
		strcpy(firstLine, "0");
	} else {
		firstLine = readLine(fileD);
	}
	int manifestLength = strlen(path+7) + strlen(hashString) + strlen(firstLine);
	char* manifestLine = malloc(manifestLength + 3);
	strcpy(manifestLine, path+7);
	strcat(manifestLine, " ");
	strcat(manifestLine, firstLine);
	printf("first line: %s\n", firstLine);
	strcat(manifestLine, " ");
	strcat(manifestLine, hashString);
	printf("full line:\n%s", manifestLine);
	while(x != 0)
	{
		x = read(fileD, buff, 1);
	}
	write(fileD, manifestLine, strlen(manifestLine));
	write(fileD, "\n", strlen("\n"));
	//printf("file desc %d\n", fd);
	//printf("%s will be added to %s\n", filename, projName);
}
/*
	Generates hash and stores it in string
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
	char* buffer = malloc(num+1);
	//char * temp = buffer;
	read(client_socket, buffer, num+1);
	printf("sent from server %s\n", buffer);
	return buffer;
}
/*
	compares server and client manifests
*/
void compareManifests(char* manifestString, char* projName) {
	//printf("The manifest from %s will be compared with\n%s", projName, manifestString);
	char* token = strtok(manifestString+1, " \n");
	int i = 0;
	while (token[i] != '$' ) {
		i++;
	}
	printf("version: %s\n", token+i+1);
	char* serverVersion = token+i+1;
	struct manifestNode * serverManifest = NULL;
	while (token != NULL) {
		token = strtok(NULL, " \n");
		char* path = token;
		token = strtok(NULL, " \n");
		char* version = token;
		token = strtok(NULL, " \n");
		char* hash = token;
	
		if (path != NULL) {
			addManifestList(&serverManifest, path, version, hash);
		}
	}
	char* path = malloc(strlen("client/") + strlen(projName) + strlen("/manifest.txt") + 1);
	strcpy(path, "client/");
	strcat(path, projName);
	strcat(path, "/manifest.txt");
	path[strlen("client/") + strlen(projName) + strlen("/manifest.txt")] = '\0';
	printf("client manifest\n");
	int fd = open(path, O_RDONLY);
	char* otherManifest = readFile(fd);
	//printf("manifestfile string %s\n", otherManifest);
	char* token2 = strtok(otherManifest+1, " \n");
	printf("version: %s\n", token2);
	char* clientVersion = token2;
	struct manifestNode * clientManifest = NULL;
	while (token2 != NULL) {
		token2 = strtok(NULL, " \n");
		char* path = token2;
		token2 = strtok(NULL, " \n");
		char* version = token2;
		token2 = strtok(NULL, " \n");
		char* hash = token2;
	
		if (path != NULL) {
			addManifestList(&clientManifest, path, version, hash);
		}
	}
	if (strcmp(serverVersion, clientVersion) == 0) {
		checkUpLoad(serverManifest, clientManifest);
	} else {
		checkModify(serverManifest, clientManifest);
		checkAdd(serverManifest, clientManifest);
		checkDelete(serverManifest, clientManifest);
	}
}
/*
	checking if file should be uploaded
*/
void checkUpLoad(struct manifestNode * serverManifest, struct manifestNode * clientManifest) {
	//printf("in check upload method\n");
	struct manifestNode *clientPtr = clientManifest;
	while (clientPtr != NULL) {
		if (checkIfPresent(clientPtr, serverManifest, "upload") == 0) {
			printf("U %s\n", clientPtr->path);	
		}
		clientPtr = clientPtr->next;
	}
}
/*
	checks if path is present 
*/
int checkIfPresent (struct manifestNode* clientNode, struct manifestNode* head, char* operation) {
	struct manifestNode* ptr = head;
	while (ptr != NULL) {
		if (strcmp(clientNode->path, ptr->path) == 0) {
			if (strcmp(operation, "upload") == 0){
				int fd = getClientFilePath(clientNode->path);
				char* string = readFile(fd);
				char* hashString = hash(string);
				//printf("serverNode: %s hashString %s\n", ptr->hash, hashString);
				if (strcmp(ptr->hash, hashString) != 0) {
					return 0;
				}
			} else if (strcmp(operation, "modify") == 0) {
				int fd = getClientFilePath(clientNode->path);
				char* string = readFile(fd);
				char* hashString = hash(string);
				if (strcmp(clientNode->hash, hashString) == 0) {
					if (strcmp(ptr->version, clientNode->version) != 0) {
						return 0;
					}
				}
			}
			return 1;
		}
		ptr = ptr->next;
	}
	if (strcmp(operation, "upload") == 0 || strcmp(operation, "add") == 0 || strcmp(operation, "delete") == 0) {
		return 0;
	}
	return 1;
}
void addManifestList(struct manifestNode ** head, char* path, char* version, char* hash) {
	struct manifestNode* temp = (struct manifestNode*)malloc(sizeof(struct manifestNode));
	temp->path = malloc(strlen(path)+1);
	strcpy(temp->path, path);
	temp->path[strlen(path)] = '\0';
	temp->version = malloc(strlen(version)+1);
	strcpy(temp->version, version);
	temp->version[strlen(version)] = '\0';
	temp->hash = malloc(strlen(hash)+1);
	strcpy(temp->hash, hash);
	temp->hash[strlen(hash)] = '\0';
	printf("the node contains %s %s %s\n", path, version, hash);
	if (*head == NULL) {
		*head = temp;
	} else {
		struct manifestNode * ptr = *head;
		while (ptr->next != NULL) {
			ptr = ptr->next;
			//counter++;
		}
		//counter++;
		ptr->next = temp;
	}
}
void checkModify(struct manifestNode * serverManifest, struct manifestNode * clientManifest) {
	struct manifestNode *clientPtr = clientManifest;
	while (clientPtr != NULL) {
		if (checkIfPresent(clientPtr, serverManifest, "modify") == 0) {
			printf("M %s\n", clientPtr->path);	
		}
		clientPtr = clientPtr->next;
	}
}
void checkAdd(struct manifestNode * serverManifest, struct manifestNode * clientManifest) {
	struct manifestNode *serverPtr = serverManifest;
	while (serverPtr != NULL) {
		if (checkIfPresent(serverPtr, clientManifest, "add") == 0) {
			printf("A %s\n", serverPtr->path);	
		}
		serverPtr = serverPtr->next;
	}
}
void checkDelete(struct manifestNode * serverManifest, struct manifestNode * clientManifest) {
	struct manifestNode *clientPtr = clientManifest;
	while (clientPtr != NULL) {
		if (checkIfPresent(clientPtr, serverManifest, "delete") == 0) {
			printf("D %s\n", clientPtr->path);	
		}
		clientPtr = clientPtr->next;
	}
}
int getClientFilePath(char* filename) {
	char* path = malloc(strlen("client/") +  strlen(filename) +1 );
	strcpy(path, "client/");
	strcat(path, filename);
	//printf("path: %s\n", path);
	int fd = open(path, O_RDONLY);
	if (fd > 0) {
		//printf("file has been successfully opened\n");
	}
	return fd;
}
