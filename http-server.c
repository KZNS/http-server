#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#define HTTP_PORT 8080
#define HTTPS_PORT 10443
#define MESSAGE200 "HTTP/1.1 200 OK\r\n"    \
                   "Content-Length: 11\r\n" \
                   "\r\n"                   \
                   "hello world"
#define MESSAGE301 "HTTP/1.1 301 Moved Permanently\r\n"        \
                   "Location: https://10.0.0.1/index.html\r\n" \
                   "\r\n"                                      \
                   ""
#define MESSAGE206 "HTTP/1.1 206 Partial Content\r\n" \
                   "Content-Length: 11\r\n"           \
                   "\r\n"                             \
                   "hello world"
#define MESSAGE404 "HTTP/1.1 404 Not Found\r\n" \
                   "Content-Length: 11\r\n"     \
                   "\r\n"                       \
                   "hello world"

int socket_listen(int port)
{
    struct sockaddr_in srvaddr;
    int locfd;
    int sockopt = 1;
    int res;

    /*创建一个套接字*/
    locfd = socket(AF_INET, SOCK_STREAM, 0);
    if (locfd < 0)
    {
        printf("create socket error! port: %d\n", port);
        exit(-1);
    }
    printf("socket ready! locfd: %d\n", locfd);

    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(port);
    srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    setsockopt(locfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int));

    /*bind, 将网络地址与端口绑定*/
    res = bind(locfd, (struct sockaddr *)&srvaddr, sizeof(srvaddr));
    if (res < 0)
    {
        printf("bind error! port: %d locfd: %d\n", port, locfd);
        close(locfd);
        exit(-1);
    }
    printf("bind ready! port: %d locfd: %d\n", port, locfd);

    /*listen, 监听端口*/
    listen(locfd, 10);
    printf("waiting...\n");
    return locfd;
}

int socket_accept(int locfd)
{
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    int clifd;

    clifd = accept(locfd, (struct sockaddr *)&cliaddr, &len);
    if (clifd < 0)
    {
        printf("accept error! locfd: %d\n", locfd);
        close(locfd);
        exit(-1);
    }

    /*输出客户机的信息*/
    char *ip = inet_ntoa(cliaddr.sin_addr);

    printf("ip %s connect to %d\n", ip, locfd);

    /*输出客户机请求的信息*/
    char buff[1024] = {0};
    int size = read(clifd, buff, sizeof(buff));

    printf("Request information: ");
    printf("%s\n", buff);
    printf("%d bytes\n", size);

    return clifd;
}

void *http_server()
{
    int locfd = socket_listen(HTTP_PORT);

    while (1)
    {
        int clifd = socket_accept(locfd);

        write(clifd, MESSAGE301, strlen(MESSAGE301));

        close(clifd);
    }

    close(locfd);
}

void *https_server()
{
    int locfd = socket_listen(HTTPS_PORT);

    while (1)
    {
        int clifd = socket_accept(locfd);

        write(clifd, MESSAGE200, strlen(MESSAGE200));

        close(clifd);
    }

    close(locfd);
}

int main()
{
    pthread_t th_http;
    pthread_t th_https;

    pthread_create(&th_http, NULL, (void *)http_server, NULL);
    pthread_create(&th_https, NULL, (void *)https_server, NULL);

    pthread_join(th_http, NULL);
    pthread_join(th_https, NULL);

    return 0;
}