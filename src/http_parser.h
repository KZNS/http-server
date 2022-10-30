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
} HTTP_parser;

HTTP_parser http_parser(char *s);
