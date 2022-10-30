typedef struct
{
    char *raw;

    enum HTTP_method
    {
        GET,
        POST,
        OTHER
    } method;
    char *path;
    char *version;

    char *host;
} HTTP_parser;

HTTP_parser http_parser(char *s);
