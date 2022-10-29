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
    int res;

    // 创建一个套接字
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        printf("! socket create error, port: %d\n", port);
        exit(-1);
    }
    printf("socket ready, port %d fd: %d\n", port, fd);

    struct sockaddr_in srvaddr;
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(port);
    srvaddr.sin_addr.s_addr = INADDR_ANY;

    // 将网络地址与端口绑定
    res = bind(fd, (struct sockaddr *)&srvaddr, sizeof(srvaddr));
    if (res < 0)
    {
        printf("! socket bind error, port: %d fd: %d\n", port, fd);
        close(fd);
        exit(-1);
    }
    printf("socket bind ready, port: %d fd: %d\n", port, fd);

    listen(fd, 10);
    printf("socket listening, port: %d fd: %d\n", port, fd);

    return fd;
}

int socket_accept(int fd)
{
    struct sockaddr_in caddr;
    socklen_t len = sizeof(caddr);
    int cfd;

    cfd = accept(fd, (struct sockaddr *)&caddr, (socklen_t *)&len);
    if (cfd < 0)
    {
        printf("! socket accept error, fd: %d\n", fd);
        close(fd);
        exit(-1);
    }
    char *ip = inet_ntoa(caddr.sin_addr);

    printf("socket accepted, fd: %d cfd: %d ip: %s\n", fd, cfd, ip);
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

SSL_CTX *ssl_ctx_init()
{
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    SSL_library_init();

    SSL_CTX *ctx = SSL_CTX_new(SSLv23_server_method());
    if (ctx == NULL)
    {
        printf("! ssl ctx new error\n");
        exit(-1);
    }
    printf("ssl ctx initialized\n");
    return ctx;
}

void ssl_ctx_config(SSL_CTX *ctx)
{
    if (SSL_CTX_use_certificate_file(ctx, "./keys/cnlab.cert", SSL_FILETYPE_PEM) <= 0)
    {
        printf("! ssl ctx 'SSL_CTX_use_certificate_file' error\n");
        exit(-1);
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "./keys/cnlab.prikey", SSL_FILETYPE_PEM) <= 0)
    {
        printf("! ssl ctx 'SSL_CTX_use_PrivateKey_file' error\n");
        exit(-1);
    }
    if (SSL_CTX_check_private_key(ctx) <= 0)
    {
        printf("! ssl ctx 'SSL_CTX_check_private_key' error\n");
        exit(-1);
    }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
    SSL_CTX_set_cipher_list(ctx, "RC4-MD5");
    SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);

    printf("ssl ctx config pass\n");
}

SSL *ssl_accept(SSL_CTX *ctx, int cfd)
{
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, cfd);

    int retcode = SSL_accept(ssl);
    if (retcode == 0)
    {
        printf("ssl accept error, code: %d\n", SSL_get_error(ssl, retcode));
        exit(-1);
    }

    printf("ssl accepted, cfd: %d\n", cfd);
    return ssl;
}

void *https_server()
{
    int fd = socket_listen(HTTPS_PORT);

    SSL_CTX *ctx = ssl_ctx_init();
    ssl_ctx_config(ctx);

    while (1)
    {
        int cfd = socket_accept(fd);
        SSL *ssl = ssl_accept(ctx, cfd);

        SSL_write(ssl, MESSAGE200, strlen(MESSAGE200));

        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(cfd);
    }

    SSL_CTX_free(ctx);

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