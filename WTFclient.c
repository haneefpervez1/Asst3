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

struct manifest{
	char * path;
	int version;
	int hash;
	struct manifest * next;
};

int configure(char*, char*);
int getPortNum();
void addFile(char*, char*);
char* readFile(int fd);
char* hash (char *);
void send_to_server(int, char *);


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
		send_to_server(network_socket, argv[1]);
		send_to_server(network_socket, argv[2]);
		//printf("%s", argv[1]);
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
	//close(network_socket);
	return 0;
}
void send_to_server(int network_socket, char * string)
{
 int len = strlen(string);
 printf("String Name: %s\n", string);
 char c[4];
 sprintf(c, "%d", len);
 printf("Length of String: %s\n", c);
 write(network_socket, c, sizeof(c));
 write(network_socket, string, sizeof(string));
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

void addFile (char* projName, char* filename) {				// still need to deal with if project doesnt exist
	
	int length = strlen(projName) + strlen(filename) + strlen("client/");
	printf("length %d\n", length);
	char* path = malloc(length+2);
	strcpy(path, "client/");
	strcat(path, projName);
	strcat(path, "/");
	strcat(path, filename);
	int fd = open(path, O_RDONLY);
	char* string = readFile(fd);
	char* hashString = hash(string);
	printf("new file: %s fd: %d\nThese are the contents: %s\n", path, fd, hashString);
	//printf("file desc %d\n", fd);
	//printf("%s will be added to %s\n", filename, projName);
}

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
