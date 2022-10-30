#include "http_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HTTP_parser http_parser(char *s)
{
    HTTP_parser http;
    http.path = NULL;
    http.version = NULL;
    http.host = NULL;
    http.range_raw = NULL;

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
        else if (strcmp(key, "Range") == 0)
        {
            http.range_raw = value;
            char *eqp = strchr(value, '=');
            char *mnp = strchr(value, '-');
            if (*(eqp + 1) != '-')
            {
                sscanf(eqp + 1, "%d", &http.range_left);
            }
            else
            {
                http.range_left = 0;
            }
            if (*(mnp + 1) != '\0')
            {
                sscanf(mnp + 1, "%d", &http.range_right);
            }
            else
            {
                http.range_right = -1;
            }
        }
        else
        {
            free(value);
        }

        s += 1; // '\n'
    }

    return http;
}
