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
    char *range_raw;
    int range_left;
    int range_right;
} HTTP_parser;

HTTP_parser http_parser(char *s);
