#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#define BUF_SIZE 1024
#define SERVER_PORT 5114
#define EPOLL_RUN_TIMEOUT -1
#define EPOLL_SIZE 10000

char message[BUF_SIZE];


int main(int argc, char *argv[])
{
        int sock, epfd;

        struct sockaddr_in addr;
        static struct epoll_event ev, events[2];
        ev.events = EPOLLIN | EPOLLET;    
        int continue_to_work = 1;
  
        if (argc != 2) {
            fprintf(stderr, "usage: %s <host>\n", argv[0]);
            return -1;
         }
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Socket creation error \n");
            return -1;
        }

        memset(&addr, '0', sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(SERVER_PORT);
        addr.sin_addr.s_addr = inet_addr(argv[1]);

        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            printf("\nConnection Failed \n");
            return -1;
        }

        if ((epfd = epoll_create(EPOLL_SIZE)) < 0) {
            printf("\nCan't create epoll\n");
            return -1;
        }
    
        ev.data.fd = sock;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev) < 0) {
            printf("\nControll Error\n");
            return -1;
        }    
        recv(sock , message, BUF_SIZE, 0);
        printf("%s",message);   
        printf("Enter 'exit' to exit\n");
        int epoll_events_count, res;
        while(continue_to_work){
                bzero(&message, BUF_SIZE);
                fgets(message, BUF_SIZE, stdin);
                if(!strcmp(message, "exit\n")){
                        break;
                       
                } else {
                    if (send(sock, message, strlen(message), 0) < 0) {
                        printf("\nCan't send data\n");
                    }
                }
                epoll_events_count = epoll_wait(epfd, events, 2, EPOLL_RUN_TIMEOUT);
                for(int i = 0; i < epoll_events_count ; i++){
                    bzero(&message, BUF_SIZE);
                    if(events[i].data.fd == sock){
                            res = recv(sock, message, BUF_SIZE, 0);
                            if(res == 0){
                                    close(sock);
                                    continue_to_work = 0;
                            }else
                                printf("%s", message);
                }
        }
    }
    close(sock);
    return 0;
}