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

    char *version = strsep(&s, "\r");
    http.version = malloc(strlen(version) + 1);
    strcpy(http.version, version);
    s += 1; // '\n'

    char key[100];
    char *value;
    char *line;
    while (1)
    {
        line = strsep(&s, "\r");
        if (strlen(line) == 0)
            break;
        strcpy(key, strsep(&line, ":"));
        line += 1; // ' '
        value = malloc(strlen(line) + 1);
        strcpy(value, line);

        if (strcmp(key, "Host") == 0)
        {
            http.host = value;
        }
        else
        {
            free(value);
        }

        s += 1; // '\n'
    }

    return http;
}
