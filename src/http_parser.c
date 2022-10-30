#include "http_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HTTP_parser http_parser(char *s)
{
    HTTP_parser http;

    http.raw = malloc(strlen(s) + 1);
    strcpy(http.raw, s);
    printf("---- http.raw\n%s----\n", http.raw);

    char *method = strsep(&s, " ");
    if (strcmp(method, "GET") == 0)
    {
        http.method = GET;
    }
    else if (strcmp(method, "POST") == 0)
    {
        http.method = POST;
    }
    else
    {
        http.method = OTHER;
        printf("http parser error, method %s\n", method);
        exit(-1);
    }

    char *path = strsep(&s, " ");
    http.path = malloc(strlen(path) + 1);
    strcpy(http.path, path);

    return http;
}
