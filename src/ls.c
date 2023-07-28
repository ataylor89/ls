#include "ls.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdint.h>

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

DirectoryList* get_directory_list(char* dirname)
{
    DirectoryList* dl;
    Metadata* metadata;
    Record** cursor;
    Record* record;
    char path[256];
    DIR* dir;
    struct dirent* dir_ent;
    struct stat file_attr;
    struct passwd* pw;
    struct group* gr;

    dl = (DirectoryList *) malloc(sizeof(DirectoryList));
    dl->dirname = dirname;

    if (preprocess(dl) != 0)
    {
        return NULL;
    }

    if ((dir = opendir(dirname)) == NULL)
    {
        return NULL;
    }

    metadata = dl->metadata;

    dl->start = (Record**) malloc(sizeof(Record*) * dl->num_records);
    cursor = dl->start;

    while ((dir_ent = readdir(dir)) != NULL)
    {
        strcpy(path, dirname);
        strcat(path, "/");
        strcat(path, dir_ent->d_name);

        if (stat(path, &file_attr) == -1)
        {
            continue;
        }

        pw = getpwuid(file_attr.st_uid);
        gr = getgrgid(file_attr.st_gid);

        cursor[0] = (Record *) malloc(sizeof(Record));
        record = cursor[0];
        record->type = dir_ent->d_type;
        record->mode = file_attr.st_mode;
        record->uid = file_attr.st_uid;
        record->gid = file_attr.st_gid;
        record->filesize = file_attr.st_size;
        strcpy(record->username, pw->pw_name);
        strcpy(record->groupname, gr->gr_name);
        strftime(record->atime, 32, "%b %d %H:%M", localtime(&file_attr.st_atime));
        strftime(record->mtime, 32, "%b %d %H:%M", localtime(&file_attr.st_mtime));
        strcpy(record->filename, dir_ent->d_name);

        cursor++;
    }

    closedir(dir);

    return dl;
}

void format_directory_list(DirectoryList* dl)
{
    Metadata* metadata;
    Record** cursor;
    Record* record;
    char* buf;
    int offset, length, i;

    metadata = dl->metadata;
    cursor = dl->start;
    offset = 0;
    length = 0;

    for (i = 0; i < dl->num_records; i++)
    {
        record = cursor[0];
        length += 11;
        length += strlen(record->username) + 1;
        length += strlen(record->groupname) + 1;
        length += metadata->filesize_field_length + 1;
        length += strlen(record->mtime) + 1;
        length += strlen(record->filename) + 1;
        cursor++;
    }

    cursor = dl->start;
    buf = (char *) malloc(length);

    for (i = 0; i < dl->num_records; i++)
    {
        record = cursor[0];

        strcat(buf, record->type == DT_DIR ? "d" : "-");
        strcat(buf, record->mode & S_IRUSR ? "r" : "-");
        strcat(buf, record->mode & S_IWUSR ? "w" : "-");
        strcat(buf, record->mode & S_IXUSR ? "x" : "-");
        strcat(buf, record->mode & S_IRGRP ? "r" : "-");
        strcat(buf, record->mode & S_IWGRP ? "w" : "-");
        strcat(buf, record->mode & S_IXGRP ? "x" : "-");
        strcat(buf, record->mode & S_IROTH ? "r" : "-");
        strcat(buf, record->mode & S_IWOTH ? "w" : "-");
        strcat(buf, record->mode & S_IXOTH ? "x" : "-");
        strcat(buf, " ");
        offset += 11;

        strcat(buf, record->username);
        strcat(buf, " ");
        offset += strlen(record->username) + 1;

        strcat(buf, record->groupname);
        strcat(buf, " ");
        offset += strlen(record->groupname) + 1;

        sprintf(buf + offset, metadata->filesize_format, record->filesize);
        strcat(buf, " ");
        offset += metadata->filesize_field_length + 1;

        strcat(buf, record->mtime);
        strcat(buf, " ");
        offset += strlen(record->mtime) + 1;

        strcat(buf, record->filename);
        strcat(buf, "\n");
        offset += strlen(record->filename) + 1;

        cursor++;
    }

    dl->buf = buf;
    dl->length = length;
}

void sort_directory_list(DirectoryList* dl)
{
    Record** records;
    Record* tmp;
    int i, j, k, size, result;

    records = dl->start;
    size = dl->num_records;

    for (i = 0; i < size - 1; i++)
    {
        k = i;

        for (j = i + 1; j < size; j++)
        {
            result = strcmp(records[k]->filename, records[j]->filename);

            if (result > 0)
            {
                k = j;
            }
        }

        if (k != i)
        {
            tmp = records[i];
            records[i] = records[k];
            records[k] = tmp;
        }
    }
}