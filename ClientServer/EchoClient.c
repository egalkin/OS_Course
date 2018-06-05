#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#define PORT 5119

int main(int argc, char const *argv[])
{
	struct sockaddr_in address;
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char buffer[1024] = {0};
	if (argc != 2) {
		fprintf(stderr, "usage: %s <host>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	printf("\n Socket creation error \n");
		return -1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]); 
	serv_addr.sin_port = htons(PORT);

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("\nConnection Failed \n");
		return -1;
	}

	printf("To exit send 'disconnect'\n");
	valread = read( sock , buffer, 1024);
	printf("%s",buffer );	

	while (1) {
		char* message;
		size_t buffsize = 255;
		getline(&message, &buffsize, stdin);
		if (!strcmp(message, "disconnect\n")) {
			break;
		}
		send(sock , message , strlen(message) , 0 );
		char recieved[255] = {0};
		valread = read( sock , recieved, 255);
		printf("%s",recieved );	
	}
	close(sock);
return 0;
}

