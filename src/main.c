#include "ls.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    Listing* listing;
    char* filename;
    char* buf;

    switch (argc)
    {
        case 1:
            filename = ".";
            break;
        case 2:
            filename = argv[1];
            break;
        default:
            printf("Usage: %s [dir_name]\n", argv[0]);
            return 0;
    }

    listing = ls(filename);
    
    if (listing->status_code != 0)
    {
        fprintf(stderr, "%s", listing->error_msg);
        return listing->status_code;
    }

    concat(listing);

    fwrite(listing->buf, 1, listing->length, stdout);

    return 0;
}