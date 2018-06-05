#include<stdlib.h>
#include<stdio.h>
#include<string.h> 
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<unistd.h>

int main(int argc , char *argv[]) {
int socket_desc , client_sock , c , read_size;
struct sockaddr_in server , client;
char client_message[1024];
int valread;

if (argc != 2) {
    fprintf(stderr, "usage: %s <host>\n", argv[0]);
    exit(EXIT_FAILURE);
}

socket_desc = socket(AF_INET , SOCK_STREAM , 0);
if (socket_desc == -1) {
    printf("Could not create socket");
}
puts("Socket created");

server.sin_family = AF_INET;
server.sin_addr.s_addr = inet_addr(argv[1]);
server.sin_port = htons(5119);

if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0) {
    perror("bind failed. Error");
    exit(EXIT_FAILURE);
}
puts("bind done");


listen(socket_desc , 3);

puts("Waiting for incoming connections...");

while(1) {
    c = sizeof(struct sockaddr_in);
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    puts("Connection accepted");


    char *message = "ECHO Server \r\n";  
    if( send(client_sock, message, strlen(message), 0) != strlen(message)) {  
        perror("send");  
    }  

    while( (valread = read( client_sock , client_message, 1024))) {
        client_message[valread] = '\0';  
        send(client_sock , client_message , strlen(client_message),0);
    }

    if(valread == 0) {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(valread == -1) {
        perror("read failed");
    }

}

return 0;
}
