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
#include <netdb.h>
#include<pthread.h>
struct manifestNode{
	char * path;
	char* version;
	char* hash;
	struct manifestNode * next;
};

struct updateNode {
	char* tag;
	char* path;
	char* hash;
	char* version;
	struct updateNode* next;
};

struct addrinfo hints, *infoptr;

int configure(char*, char*);
char* readConfigure();
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
int openUpdate(char* );
char* getUpgradeList(char* ); 
void addUpgradeList(struct updateNode ** , char* , char* , char*,char* );
void deleteFromManifest(struct updateNode* , struct manifestNode** );
void updateManifest(struct updateNode* , struct manifestNode** );
void overWriteMan (struct manifestNode** , char* , char* , char* ) ;
char* requestFiles(struct updateNode* );
int checkDir(char *);
void writeFile(char* , char* );
void *get_in_addr(struct sockaddr *);
int findHighestVersion(struct updateNode* );
void writeNewManifest(struct manifestNode*, char*, struct updateNode*);

void *get_in_addr(struct sockaddr*sa)
{
 	if(sa->sa_family == AF_INET)
 	{
 	 return & (((struct sockaddr_in*)sa)->sin_addr);
 	}
 	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main (int args, char** argv) {
	char client [100] = "client/";
	if (strcmp(argv[1], "configure") == 0) {
		printf("configure %d\n", configure(argv[2], argv[3]));
	}
	if (strcmp(argv[1], "add") == 0) {
		addFile(argv[2], argv[3]);
		return 0;
	}
// ----------------------------------------------------------------------------------> SOCKET CREATION
    int network_socket;  
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    char* configureString = readConfigure();
	char* token = strtok(configureString, " ");
	char* hostName = token;
	printf("HostName: %s\n", hostName);
	token = strtok(NULL, " ");
	int PORT = atoi(token);
	PORT= PORT/10;
	char port[strlen(token)-1];
	sprintf(port, "%d", PORT);
	printf("Port: %s\n", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags=AI_PASSIVE;
    if ((rv = getaddrinfo(hostName, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((network_socket = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(network_socket, p->ai_addr, p->ai_addrlen) == -1) {
            close(network_socket);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure
    
    printf("client: received");
	
// ------------------------------------------------------------------------------------------------------------------> SOCKET CREATION
	if (strcmp(argv[1], "checkout") == 0){				// If command is checkout
		//command = argv[1];
		//strcat(command, argv[2]);
		char path[7+strlen(argv[2])];
		strcpy(path, "client/");
	  	strcat(path,argv[2]);
		if (checkDir(path)!=0) 
		{
		send_to_server(network_socket, argv[1]);		// writes to server socket
		send_to_server(network_socket, argv[2]);
		char* string = READ(network_socket);			// reads from socket
		mkdir(path, 0700);
		read_string(string);
		//printf("%s", argv[1]
		}
	}
	
	if (strcmp(argv[1], "create")==0)					// if command is create
	{

	 char direct[100];
	 char path [100];
	 send_to_server(network_socket, argv[1]);
	 send_to_server(network_socket, argv[2]);
	 char* response = READ(network_socket);
	
	 if(strcmp(response, "created") == 0 )
	 {
	  strcpy(direct, client);
	  strcat(direct, argv[2]);						// creating new project and making emptry manifest
	  mkdir(direct, 0700);
	  strcpy(path, direct);
	  strcat(path, "/.Manifest");
	  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	  open(path,O_RDWR | O_CREAT, mode);
	 }
	
	}
	if (strcmp(argv[1], "update") == 0) {			// if command is update
		send_to_server(network_socket, argv[1]);
		send_to_server(network_socket, argv[2]);
		char* string = READ(network_socket);
		compareManifests(string, argv[2]);
	}
	if (strcmp(argv[1], "upgrade") == 0) {			// if command is upgrade
		char* requestMessage = getUpgradeList(argv[2]);
		if (requestMessage == NULL) {
			printf("Error: No .Update file present\n");
			return 0;
		}
		printf("requestMessage %s\n", requestMessage);	// getting files that need to be added or modified
		send_to_server(network_socket, argv[1]);
		send_to_server(network_socket, requestMessage);
		char* newFiles = READ(network_socket);
		printf("From server: %s", newFiles);
		char *token = strtok(newFiles, ":");
		token = strtok(NULL, ":");
		while (token != NULL) {
			token = strtok(NULL, ":");
			token = strtok(NULL, ":");
			char* filename = token;
			token = strtok(NULL, ":");
			token = strtok(NULL, ":");
			char* contents = token;
			if (filename != NULL && contents != NULL) {
				writeFile(filename, contents);			// writing new files
			}
		}
		char* updatePath = malloc(strlen("client/") + strlen(argv[2]) + strlen("/.Update") + 1);
		strcpy(updatePath, "client/");
		strcat(updatePath, argv[2]);
		strcat(updatePath, "/.Update");
		updatePath[strlen("client/") + strlen(argv[2]) + strlen("/.Update")] = '\0';
		remove(updatePath);								// removing .Update
	}
	if (strcmp(argv[1], "destroy") == 0) {				// if command is update
		send_to_server(network_socket, argv[1]);
		send_to_server(network_socket, argv[2]);
	}
	close(network_socket);
	return 0;
}
int checkDir(char * path)			// checks if project has already been created
{
 	struct stat st = {0};
	if (stat(path, &st) != -1)
	{
    	printf("Error:Project name already exists\n");
    	return 0;
	}
  return 1;
}
/*
	sends message to server
*/
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
/*
	configures file
*/
int configure(char* hostname, char* port) {
	if (access(".configure", F_OK) != -1) {
		remove(".configure");
	//	return 1;
	} 
	printf("hostname: %s port: %s \n", hostname, port);
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
 	int configure_file = creat(".configure", mode);
	write(configure_file, hostname, strlen(hostname));
	write(configure_file, " ", strlen(" "));
	write(configure_file, port, strlen(port));
	return -1;
}
/*
	reads configure file
*/
char* readConfigure() {
	if (access(".configure", F_OK) != -1) {
		int fd = open(".configure", O_RDONLY);
		char* string = readFile(fd);
		return string;
	}
	return NULL;
}
/*
	reads file and returns contents as a string
*/
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
/*
	read string from server and creates files from it
*/
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
	 	 printf("File %s Created\n", filename);
	 	 token = strtok(NULL, ":");	// Bytes to write
	 	 int bytes = atoi(token);
	 	 token = strtok(NULL, ":");	// What to Write
	 	 write(fd, token, bytes);
	 	 printf("Content %s written\n", token);
	 	 close(fd);
	 	 fileNum--;
		}
 	}
}
/*
 adds files to manifestt
*/
void addFile (char* projName, char* filename) {				
	
	int length = strlen(projName) + strlen(filename) + strlen("client/");
	char* path = malloc(length+2);
	strcpy(path, "client/");
	strcat(path, projName);
	strcat(path, "/");
	strcat(path, filename);
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		printf("Error: %s does not exist\n", projName);
		return;
	}
	char* string = readFile(fd);
	char* hashString = hash(string);

	char* manifestPath = malloc(strlen(projName) + 14 +strlen("client/"));
	strcpy(manifestPath, "client/");
	strcat(manifestPath, projName);
	strcat(manifestPath, "/.Manifest");
	
	int fileD = open(manifestPath, O_RDWR);
	
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
	strcat(manifestLine, " ");
	strcat(manifestLine, hashString);
	printf("full line:\n%s", manifestLine);
	while(x != 0)
	{
		x = read(fileD, buff, 1);
	}
	write(fileD, manifestLine, strlen(manifestLine));
	write(fileD, "\n", strlen("\n"));
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
	char* token = strtok(manifestString+1, " \n");
	int i = 0;
	while (token[i] != '$' ) {
		i++;
	}
	char* serverVersion = token+i+1;
	struct manifestNode * serverManifest = NULL;
	while (token != NULL) {
		token = strtok(NULL, " \n");
		char* path = token;
		token = strtok(NULL, " \n");
		char* version = token;
		token = strtok(NULL, " \n");
		char* hash = token;
	
		if (path != NULL && version != NULL && hash != NULL) {
			addManifestList(&serverManifest, path, version, hash);
		}
	}
	char* path = malloc(strlen("client/") + strlen(projName) + strlen("/.Manifest") + 1);
	strcpy(path, "client/");
	strcat(path, projName);
	strcat(path, "/.Manifest");
	path[strlen("client/") + strlen(projName) + strlen("/.Manifest")] = '\0';
	int fd = open(path, O_RDONLY);
	char* otherManifest = readFile(fd);
	char* token2 = strtok(otherManifest+1, " \n");
	char* clientVersion = token2;
	struct manifestNode * clientManifest = NULL;
	while (token2 != NULL) {
		token2 = strtok(NULL, " \n");
		char* path = token2;
		token2 = strtok(NULL, " \n");
		char* version = token2;
		token2 = strtok(NULL, " \n");
		char* hash = token2;
	
		if (path != NULL && version != NULL && hash != NULL) {
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
	free(serverManifest);
	free(clientManifest);
}

/*
	checking if file should be uploaded
*/
void checkUpLoad(struct manifestNode * serverManifest, struct manifestNode * clientManifest) {
	//printf("in check upload method\n");
	struct manifestNode *clientPtr = clientManifest;
	while (clientPtr != NULL) {
		if (checkIfPresent(clientPtr, serverManifest, "upload") == 0) {
			//printf("this is the client ptr path: %s\n", clientPtr->path);
			printf("U %s %s\n", clientPtr->path, clientPtr->hash);	
			//return;
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
/*
	adds node to manifst ll
*/
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
	if (*head == NULL) {
		*head = temp;
	} else {
		struct manifestNode * ptr = *head;
		while (ptr->next != NULL) {
			ptr = ptr->next;
		}
		ptr->next = temp;
	}
}
/*
	checks if a file should be modified
*/
void checkModify(struct manifestNode * serverManifest, struct manifestNode * clientManifest) {
	struct manifestNode *clientPtr = clientManifest;
	while (clientPtr != NULL) {
		if (checkIfPresent(clientPtr, serverManifest, "modify") == 0) {
			struct manifestNode* serverPtr = serverManifest;
			while (strcmp(clientPtr->path, serverPtr->path) != 0) {
				serverPtr = serverPtr->next;
			}
			printf("M %s %s\n", clientPtr->path, serverPtr->hash);
			int fd = openUpdate(clientPtr->path);
			char buff[1];
			int x = read(fd, buff, 1);
			while (x != 0) {
				x = read(fd, buff, 1);
			}
			write(fd, "M ", strlen("M "));	
			write(fd, clientPtr->path, strlen(clientPtr->path));
			write(fd, " ", strlen(" "));
			write(fd, serverPtr->hash, strlen(serverPtr->hash));
			write(fd, " ", strlen(" "));
			write(fd, serverPtr->version, strlen(serverPtr->version));
			write(fd, "\n", strlen("\n"));
		}
		clientPtr = clientPtr->next;
	}
}
/*
	checks if a file should be added
*/
void checkAdd(struct manifestNode * serverManifest, struct manifestNode * clientManifest) {
	struct manifestNode *serverPtr = serverManifest;
	while (serverPtr != NULL) {
		if (checkIfPresent(serverPtr, clientManifest, "add") == 0) {
			printf("A %s %s\n", serverPtr->path, serverPtr->hash);	
			int fd = openUpdate(serverPtr->path);
			char buff[1];
			int x = read(fd, buff, 1);
			while (x != 0) {
				x = read(fd, buff, 1);
			}
			write(fd, "A ", strlen("A "));	
			write(fd, serverPtr->path, strlen(serverPtr->path));
			write(fd, " ", strlen(" "));
			write(fd, serverPtr->hash, strlen(serverPtr->hash));
			write(fd, " ", strlen(" "));
			write(fd, serverPtr->version, strlen(serverPtr->version));
			write(fd, "\n", strlen("\n"));
		}
		serverPtr = serverPtr->next;
	}
}
/*
	checks if file should be deleted
*/
void checkDelete(struct manifestNode * serverManifest, struct manifestNode * clientManifest) {
	struct manifestNode *clientPtr = clientManifest;
	while (clientPtr != NULL) {
		if (checkIfPresent(clientPtr, serverManifest, "delete") == 0) {
			printf("D %s %s\n", clientPtr->path, clientPtr->hash);
			int fd = openUpdate(clientPtr->path);
			char buff[1];
			int x = read(fd, buff, 1);
			while (x != 0) {
				x = read(fd, buff, 1);
			}
			write(fd, "D ", strlen("D "));	
			write(fd, clientPtr->path, strlen(clientPtr->path));
			write(fd, " ", strlen(" "));
			write(fd, clientPtr->hash, strlen(clientPtr->hash));
			write(fd, " ", strlen(" "));
			write(fd, clientPtr->version, strlen(clientPtr->version));
			write(fd, "\n", strlen("\n"));
		}
		clientPtr = clientPtr->next;
	}
}
/*
	appends client/ to file path and returns file desc
*/
int getClientFilePath(char* filename) {
	char* path = malloc(strlen("client/") +  strlen(filename) +1 );
	strcpy(path, "client/");
	strcat(path, filename);
	int fd = open(path, O_RDONLY);
	return fd;
}
/*
	creates .Update file and returns file desc
*/
int openUpdate(char* filename) {
	int i = 0;
	while(filename[i] != '/') {
		i++;
	}
	char* projName = malloc(i+1);
	strncpy(projName, filename, i);
	char* path = malloc(strlen("client/") +  strlen(projName) + strlen("/.Update") + 1);
	strcpy(path, "client/");
	strcat(path, projName);
	strcat(path, "/.Update");
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	int fd = open(path,O_RDWR | O_CREAT, mode);
	return fd;
}
char* getUpgradeList(char* projName) {
	char* updatePath = malloc(strlen("client/") + strlen(projName) + strlen("/.Update") + 1);
	strcpy(updatePath, "client/");
	strcat(updatePath, projName);
	strcat(updatePath, "/.Update");
	if (access(updatePath, F_OK) == -1) {
		return NULL;
	}
	int fd = open(updatePath, O_RDONLY);
	char* contents = readFile(fd);
	char* token = strtok(contents, " \n");
	char* tag = token;
	token = strtok(NULL, " \n");
	char* path = token;
	token = strtok(NULL, " \n");
	char* hash = token;
	token = strtok(NULL, " \n");
	char* version = token;
	struct updateNode * upgradeList = NULL;
	addUpgradeList(&upgradeList, tag, path, hash, version);
	while (token != NULL) {
		token = strtok(NULL, " \n");
		tag = token;
		token = strtok(NULL, " \n");
		path = token;
		token = strtok(NULL, " \n");
		hash = token;
		token = strtok(NULL, " \n");
		version = token;
	
		if (tag != NULL && hash != NULL && path != NULL && version != NULL) {
			addUpgradeList(&upgradeList, tag, path, hash, version);
		}
	
	}
	char* manifestPath = malloc(strlen("client/") + strlen(projName) + strlen("/.Manifest") + 1);
	strcpy(manifestPath, "client/");
	strcat(manifestPath, projName);
	strcat(manifestPath, "/.Manifest");
	manifestPath[strlen("client/") + strlen(projName) + strlen("/.Manifest")] = '\0';
	int manfd = open(manifestPath, O_RDONLY);
	char* otherManifest = readFile(manfd);
	char* token2 = strtok(otherManifest+1, " \n");
	struct manifestNode * clientManifest = NULL;
	while (token2 != NULL) {
		token2 = strtok(NULL, " \n");
		char* path = token2;
		token2 = strtok(NULL, " \n");
		char* version = token2;
		token2 = strtok(NULL, " \n");
		char* hash = token2;
		if (path != NULL && version != NULL && hash != NULL) {
			addManifestList(&clientManifest, path, version, hash);
		}
	}
	deleteFromManifest(upgradeList, &clientManifest);
	updateManifest(upgradeList, &clientManifest);
	char* message = requestFiles(upgradeList);
	writeNewManifest(clientManifest, projName, upgradeList);
	free(clientManifest);
	free(upgradeList);
	return message;
}
/*
	adds node to upgrade list
*/
void addUpgradeList(struct updateNode ** head, char* tag, char* path, char* hash, char* version) {
	struct updateNode* temp = (struct updateNode*)malloc(sizeof(struct updateNode));
	temp->tag = malloc(strlen(tag)+1);
	strcpy(temp->tag, tag);
	temp->tag[strlen(tag)] = '\0';
	temp->path = malloc(strlen(path)+1);
	strcpy(temp->path, path);
	temp->path[strlen(path)] = '\0';
	temp->hash = malloc(strlen(hash)+1);
	strcpy(temp->hash, hash);
	temp->hash[strlen(hash)] = '\0';
	temp->version = malloc(strlen(version)+1);
	strcpy(temp->version, version);
	temp->version[strlen(version)] = '\0';
	if (*head == NULL) {
		*head = temp;
	} else {
		struct updateNode * ptr = *head;
		while (ptr->next != NULL) {
			ptr = ptr->next;
		}
		ptr->next = temp;
	}
	
}
void deleteFromManifest(struct updateNode* upgradeList, struct manifestNode** manifestList) {
	struct updateNode* updatePtr = upgradeList;
	char* fileToBeDeleted;
	while (updatePtr != NULL) {
		if (strcmp(updatePtr->tag, "D") == 0) {
			fileToBeDeleted = malloc(strlen(updatePtr->path)+1);
			fileToBeDeleted = updatePtr->path;
		}
		updatePtr = updatePtr->next;
	}

	struct manifestNode * manifestCur = *manifestList;
	struct manifestNode * manifestPrev = NULL;
	if (manifestCur != NULL && strcmp(manifestCur->path, fileToBeDeleted) == 0) {
		*manifestList = manifestCur->next;
		free(manifestCur);
		return;
	}
	while (manifestCur != NULL && strcmp(manifestCur->path, fileToBeDeleted) != 0) {
		manifestPrev = manifestCur;
		manifestCur = manifestCur->next;
	}
	manifestPrev->next = manifestCur->next;
	free(manifestCur);
}
/*
	updates manifest ll
*/
void updateManifest(struct updateNode* upgradeList, struct manifestNode** manifestList) {
	struct updateNode* updatePtr = upgradeList;
	while (updatePtr != NULL) {
		if (strcmp(updatePtr->tag, "A") == 0) {
			addManifestList(manifestList, updatePtr->path, updatePtr->version, updatePtr->hash);
		} else if (strcmp(updatePtr->tag, "M") == 0) {
			overWriteMan(manifestList, updatePtr->path, updatePtr->version, updatePtr->hash);
		}
		updatePtr = updatePtr->next;
	}
}
/*
	overwrites manifest ll
*/
void overWriteMan (struct manifestNode** manifestList, char* path, char* version, char* hash) {
	struct manifestNode* ptr = *manifestList;
	while (strcmp(path, ptr->path) != 0) {
		ptr = ptr->next;
	}
	ptr->version = malloc(strlen(version)+1);
	strcpy(ptr->version, version);
	ptr->version[strlen(version)] = '\0';
	strcpy(ptr->hash, hash);
}
/*
	generates string to send to client to get string
*/
char* requestFiles(struct updateNode* head) {	
	struct updateNode* ptr = head;
	int counter = 0;
	int length = 0;
	while (ptr != NULL) {
		if (strcmp(ptr->tag, "A") == 0 || strcmp(ptr->tag, "M") == 0) {
			length += strlen(ptr->path) + 1;
			counter++;
		}
		ptr = ptr->next;
	}
	char buffer[10];
	sprintf(buffer, "%d", counter);
	char* clientMessage = malloc(length + strlen(buffer)+3);
	strcpy(clientMessage, "r:");
	strcat(clientMessage, buffer);
	ptr = head;
	while (ptr != NULL) {
		if (strcmp(ptr->tag, "A") == 0 || strcmp(ptr->tag, "M") == 0) {
			strcat(clientMessage, ":");
			strcat(clientMessage, ptr->path);			
		}
		ptr = ptr->next;
	}
	return clientMessage;
}
void freeManifestList(struct manifestNode* head) {
	struct manifestNode* ptr;
	while (head != NULL) {
		ptr = head;
		head = head->next;
		free(ptr);
	}
}
void freeUpdateList(struct updateNode* head) {
	struct updateNode* ptr;
	while (head != NULL) {
		ptr = head;
		head = head->next;
		free(ptr);
	}
}
/*
	wites or rewrites file
*/
void writeFile(char* filename, char* contents) {
	char* path = malloc(strlen("client/") +  strlen(filename) +1 );
	strcpy(path, "client/");
	strcat(path, filename);
	if (access(path, F_OK) != -1) {
		remove(path);
		mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		int fd = open(path,O_RDWR | O_CREAT, mode);
		write(fd, contents, strlen(contents));
	} else {
		mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		int fd = open(path,O_RDWR | O_CREAT, mode);
		write(fd, contents, strlen(contents));
	}
	
}
/*
	rewrites manifest
*/
void writeNewManifest(struct manifestNode* manList, char* projName, struct updateNode* updateList) {
	char* projPath = malloc(strlen("client/") + strlen(projName) + strlen("./Manifest") + 1);
	strcpy(projPath, "client/");
	strcat(projPath, projName);
	strcat(projPath, "/.Manifest");
	projPath[strlen("client/") + strlen(projName) + strlen("./Manifest")] = '\0';
	int highestVersion = findHighestVersion(updateList);
	char buffer[10];
	sprintf(buffer, "%d", highestVersion);
	remove(projPath);
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	int fd = open(projPath, O_RDWR | O_CREAT, mode);
	write(fd, "$", strlen("$"));
	write(fd, buffer, strlen(buffer));
	write(fd, "\n", strlen("\n"));
	struct manifestNode* ptr = manList;
	while (ptr != NULL) {
		write(fd, ptr->path, strlen(ptr->path));
		write(fd, " ", strlen(" "));
		write(fd, ptr->version, strlen(ptr->version));
		write(fd, " ", strlen(" "));
		write(fd, ptr->hash, strlen(ptr->hash));
		write(fd, "\n", strlen("\n"));
		ptr = ptr->next;
	}
	
}
int findHighestVersion(struct updateNode* manList) {
	struct updateNode* ptr = manList;
	int highestVersion = 0;
	while (ptr != NULL) {
		int currVersion = atoi(ptr->version);
		if (currVersion > highestVersion) {
			highestVersion = currVersion;
		}
		ptr = ptr->next;
	}
	return highestVersion;
}
