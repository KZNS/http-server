#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include <pthread.h>

#include <openssl/ssl.h>

#define HTTP_PORT 8080
#define HTTPS_PORT 10443
#define BUF_SIZE 8192
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
    int fd;
    int sockopt = 1;
    int res;

    /*创建一个套接字*/
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        printf("create socket error! port: %d\n", port);
        exit(-1);
    }
    printf("socket ready! fd: %d\n", fd);

    struct sockaddr_in srvaddr;
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(port);
    srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int));

    /*bind, 将网络地址与端口绑定*/
    res = bind(fd, (struct sockaddr *)&srvaddr, sizeof(srvaddr));
    if (res < 0)
    {
        printf("bind error! port: %d fd: %d\n", port, fd);
        close(fd);
        exit(-1);
    }
    printf("bind ready! port: %d fd: %d\n", port, fd);

    listen(fd, 10);
    printf("waiting...\n");

    return fd;
}

int socket_accept(int fd)
{
    struct sockaddr_in caddr;
    socklen_t len = sizeof(caddr);
    int cfd;

    cfd = accept(fd, (struct sockaddr *)&caddr, &len);
    if (cfd < 0)
    {
        printf("accept error! fd: %d\n", fd);
        close(fd);
        exit(-1);
    }

    /*输出客户机的信息*/
    char *ip = inet_ntoa(caddr.sin_addr);

    printf("ip %s connect to %d\n", ip, fd);

    /*输出客户机请求的信息*/
    char buff[BUF_SIZE] = {0};
    int size = read(cfd, buff, sizeof(buff));

    printf("Request information: ");
    printf("%s\n", buff);
    printf("%d bytes\n", size);

    return cfd;
}

void *http_server()
{
    int fd = socket_listen(HTTP_PORT);

    while (1)
    {
        int cfd = socket_accept(fd);

        write(cfd, MESSAGE301, strlen(MESSAGE301));

        close(cfd);
    }

    close(fd);
}

SSL_CTX *SSL_CTX_init()
{
    SSL_load_error_strings();
    SSLeay_add_ssl_algorithms();
    OpenSSL_add_all_algorithms();
    SSL_library_init();

    SSL_CTX *ctx = SSL_CTX_new(SSLv23_server_method());
    if (ctx == NULL)
    {
        printf("SSL_CTX_new error!\n");
        exit(-1);
    }
    return ctx;
}

void SSL_CTX_load_key(SSL_CTX *ctx)
{
    if (SSL_CTX_use_certificate_file(ctx, "keys/cnlab.cert", SSL_FILETYPE_PEM) <= 0)
    {
        printf("SSL_CTX_use_certificate_file fail\n");
        exit(-1);
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "keys/cnlab.prikey", SSL_FILETYPE_PEM) <= 0)
    {
        printf("SSL_CTX_use_PrivateKey_file fail\n");
        exit(-1);
    }
    if (SSL_CTX_check_private_key(ctx) <= 0)
    {
        printf("SSL key check fail\n");
        exit(-1);
    }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0);
}

void *https_server()
{
    SSL_CTX *ctx = SSL_CTX_init();
    SSL_CTX_load_key(ctx);

    int fd = socket_listen(HTTPS_PORT);

    char buff[BUF_SIZE] = {0};
    int retcode;
    while (1)
    {
        int cfd = socket_accept(fd);
        // int cfd = accept(fd, 0, 0);
        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, cfd);
        SSL_set_accept_state(ssl);

        printf("SSL_accept linking...\n");
        retcode = SSL_accept(ssl);
        if (retcode <= 0)
        {
            printf("in\n");
            printf("SSL_accept error: %d\n", SSL_get_error(ssl, retcode));
            exit(-1);
        }
        printf("SSL_accept succeeded\n");

        bzero(buff, BUF_SIZE);
        strcpy(buff, MESSAGE200);
        SSL_write(ssl, buff, strlen(buff));

        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(cfd);
        SSL_CTX_free(ctx);
    }

    close(fd);
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