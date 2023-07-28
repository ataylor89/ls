#include "preprocessor.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

int num_digits(long filesize)
{
    int count = 0;

    while (filesize > 0)
    {
        count++;
        filesize /= 10;
    }

    return count;
}

int preprocess(DirectoryList* dl)
{
    Metadata* metadata;
    DIR* dir;
    struct dirent* dir_ent;
    struct stat file_attr;
    char filename[256];

    dl->metadata = (Metadata *) malloc(sizeof(Metadata));
    metadata = dl->metadata;

    if ((dir = opendir(dl->dirname)) == NULL)
    {
        return 1;
    }

    while ((dir_ent = readdir(dir)) != NULL)
    {
        sprintf(filename, "%s/%s", dl->dirname, dir_ent->d_name);

        if (stat(filename, &file_attr) == -1)
        {
            continue;
        }

        if (file_attr.st_size > metadata->max_filesize)
        {
            metadata->max_filesize = file_attr.st_size;
        }

        dl->num_records++;
    }

    metadata->filesize_field_length = num_digits(metadata->max_filesize);
    sprintf(metadata->filesize_format, "%%%dlld", metadata->filesize_field_length);

    if (closedir(dir) != 0)
    {
        return 1;
    }

    return 0;
}