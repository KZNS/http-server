#include <stdio.h>
#include <stdlib.h>

int read_file(char *file_name, char **ret_buf)
{
    FILE *f = fopen(file_name, "r");
    int file_size;
    if (f)
    {
        printf("opened\n");
        fseek(f, 0, SEEK_END);
        file_size = ftell(f);
        fseek(f, 0, SEEK_SET);
        *ret_buf = malloc(file_size + 1);
        fread(*ret_buf, sizeof(char), file_size, f);
        return file_size;
    }
    else
    {
        return -1;
    }
}