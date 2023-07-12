#include "ls.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    struct DirectoryListing* dl;

    if (argc == 1)
    {
        dl = ls(".");
        printf("%s", dl->status_code == 0 ? dl->buf : dl->error_msg);
        return dl->status_code;
    }

    else if (argc == 2)
    {
        dl = ls(argv[1]);
        printf("%s", dl->status_code == 0 ? dl->buf : dl->error_msg);
        return dl->status_code;
    }

    else
    {
        printf("Usage: %s [dir_name]\n", argv[0]);
        return 0;
    }
}