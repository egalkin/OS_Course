
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_PORT    5114
#define MAX_CONN        16
#define MAX_EVENTS      32
#define BUF_SIZE        1024
#define MAX_LINE        256

static void epoll_ctl_add(int epfd, int fd, uint32_t events)
{
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("epoll_ctl()\n");
        exit(1);
    }
}

static void set_sockaddr(struct sockaddr_in *addr, char* ip)
{
    bzero((char *)addr, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(ip);
    addr->sin_port = htons(DEFAULT_PORT);
}

static int setnonblocking(int sockfd)
{
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK) == -1) {
        return -1;
    }
    return 0;
}


int main(int argc, char *argv[])
{
    int i;
    int n;
    int epfd;
    int nfds;
    int listen_sock;
    int conn_sock;
    int socklen;
    char buf[BUF_SIZE];
    struct sockaddr_in srv_addr;
    struct sockaddr_in cli_addr;
    struct epoll_event events[MAX_EVENTS];

   if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Could not create listen socket");
        return -1;
    }
    puts("socket created");

    set_sockaddr(&srv_addr, argv[1]);
    if (bind(listen_sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0) {
        perror("bind failed. Error");
        exit(EXIT_FAILURE);
    }
    puts("bind done");


    setnonblocking(listen_sock);
    listen(listen_sock, MAX_CONN);
    epfd = epoll_create(1);

    epoll_ctl_add(epfd, listen_sock, EPOLLIN | EPOLLOUT | EPOLLET);
    socklen = sizeof(cli_addr);
    puts("Waiting for incoming connections...");
    while(1) {
        nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (i = 0; i < nfds; i++) {
            if (events[i].data.fd == listen_sock) {
                conn_sock = accept(listen_sock,(struct sockaddr *)&cli_addr,&socklen);
                inet_ntop(AF_INET, (char *)&(cli_addr.sin_addr),buf, sizeof(cli_addr));

                printf("Connected with %s:%d\n", buf, ntohs(cli_addr.sin_port));
                char *message = "ECHO Server \r\n";  
                if( send(conn_sock, message, strlen(message), 0) != strlen(message)) {  
                    perror("send");  
                }  
                setnonblocking(conn_sock);
                epoll_ctl_add(epfd, conn_sock,
                          EPOLLIN | EPOLLET | EPOLLRDHUP |
                          EPOLLHUP);
            } else if (events[i].events & EPOLLIN) {
                while (1) {
                    bzero(buf, sizeof(buf));
                    n = read(events[i].data.fd, buf,
                         sizeof(buf));
                    if (n <= 0) {
                        break;
                    } else {
                        write(events[i].data.fd, buf,
                              strlen(buf));
                    }
                }
            } else {
                printf("Unexpected\n");
            }
            if (events[i].events & (EPOLLRDHUP | EPOLLHUP)) {
                printf("Connection closed\n");
                epoll_ctl(epfd, EPOLL_CTL_DEL,
                      events[i].data.fd, NULL);
                close(events[i].data.fd);
                continue;
            }
        }
    }
    return 0;
}