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
int getPortNum();

int main (int args, char** argv) {

	if (strcmp(argv[1], "configure") == 0) {
		//char configureMessage[20] = "configure:2:tst.txt";
		//write(network_socket, &configureMessage, sizeof(configureMessage));
		printf("configure %d\n", configure(argv[2], argv[3]));
	}
	char client [100] = "client/";
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
	char client_message[256] = "test.txt\n";
	read(network_socket, &server_response, sizeof(server_response));
	printf("The server sent the data: %s\n", server_response);
	write(network_socket, &client_message, sizeof(client_message));
	else if (strcmp(argv[1], "create")==0)
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
	//close(network_socket);
	return 0;
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
int create (char * projectname)
{
	return 1;
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
