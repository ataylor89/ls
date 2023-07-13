#include "ls.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    DirectoryListing* dl;
    char* filename;

    switch (argc)
    {
        case 1:
            filename = ".";
            break;
        case 2:
            filename = argv[1];
            break;
        default:
            printf("Usage: %s [path]\n", argv[0]);
            return 0;
    }
    
    if ((dl = get_directory_listing(filename)) == NULL)
    {
        fprintf(stderr, "Error getting directory listing for %s\n", filename);
        return 1;
    }

    format_directory_listing(dl);

    fwrite(dl->buf, 1, dl->length, stdout);

    return 0;
}