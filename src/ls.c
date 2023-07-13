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

int num_digits(long long filesize)
{
    int count = 0;

    while (filesize > 0)
    {
        count++;
        filesize /= 10;
    }

    return count;
}

void preprocess(Listing* listing)
{
    Metadata* metadata;
    DIR* dir;
    struct dirent* dir_ent;
    struct stat file_attr;
    char filename[200];

    listing->metadata = (Metadata *) malloc(sizeof(Metadata));
    metadata = listing->metadata;

    if ((dir = opendir(listing->dirname)) == NULL)
    {
        listing->status_code = 1;
        listing->error_msg = "Error opening directory.\n";
        return;
    }

    while ((dir_ent = readdir(dir)) != NULL)
    {
        sprintf(filename, "%s/%s", listing->dirname, dir_ent->d_name);

        if (stat(filename, &file_attr) == -1)
        {
            continue;
        }

        if (file_attr.st_size > metadata->max_filesize)
        {
            metadata->max_filesize = file_attr.st_size;
        }

        listing->num_entries++;
    }

    metadata->filesize_field_length = num_digits(metadata->max_filesize);
    sprintf(metadata->filesize_format, "%%%dlld", metadata->filesize_field_length);

    if (closedir(dir) != 0)
    {
        listing->status_code = 1;
        listing->error_msg = "Error closing directory.\n";
    }
}

Listing* ls(char* dirname)
{
    Listing* listing;
    Metadata* metadata;
    Entry** cursor;
    Entry* entry;
    char path[200];
    DIR* dir;
    struct dirent* dir_ent;
    struct stat* file_attr;
    struct passwd* pw;
    struct group* gr;
    char timestamp[20];
    int length;

    listing = (Listing *) malloc(sizeof(Listing));
    listing->dirname = dirname;
    listing->error_msg = (char *) malloc(100);

    preprocess(listing);

    if (listing->status_code != 0)
    {
        return listing;
    }

    if ((dir = opendir(dirname)) == NULL)
    {
        listing->status_code = 1;
        listing->error_msg = "Error opening directory.\n";
        return listing;
    }

    metadata = listing->metadata;

    listing->start = (Entry**) malloc(sizeof(Entry*) * listing->num_entries);
    cursor = listing->start;

    file_attr = (struct stat *) malloc(sizeof(struct stat));

    while ((dir_ent = readdir(dir)) != NULL)
    {
        *cursor = (Entry *) malloc(sizeof(Entry));
        entry = *cursor;
        entry->filename = dir_ent->d_name;
        sprintf(path, "%s/%s", dirname, dir_ent->d_name);

        if (stat(path, file_attr) == -1)
        {
            continue;
        }

        pw = getpwuid(file_attr->st_uid);
        gr = getgrgid(file_attr->st_gid);
        strftime(timestamp, 20, "%b %d %H:%M", localtime(&file_attr->st_mtime));

        length = 11;
        length += strlen(pw->pw_name) + 1;
        length += strlen(gr->gr_name) + 1;
        length += metadata->filesize_field_length + 1;
        length += strlen(timestamp) + 1;
        length += strlen(dir_ent->d_name) + 1;

        entry->length = length;
        entry->buf = (char *) malloc(entry->length);

        strcat(entry->buf, dir_ent->d_type == DT_DIR ? "d" : "-");
        strcat(entry->buf, file_attr->st_mode & S_IRUSR ? "r" : "-");
        strcat(entry->buf, file_attr->st_mode & S_IWUSR ? "w" : "-");
        strcat(entry->buf, file_attr->st_mode & S_IXUSR ? "x" : "-");
        strcat(entry->buf, file_attr->st_mode & S_IRGRP ? "r" : "-");
        strcat(entry->buf, file_attr->st_mode & S_IWGRP ? "w" : "-");
        strcat(entry->buf, file_attr->st_mode & S_IXGRP ? "x" : "-");
        strcat(entry->buf, file_attr->st_mode & S_IROTH ? "r" : "-");
        strcat(entry->buf, file_attr->st_mode & S_IWOTH ? "w" : "-");
        strcat(entry->buf, file_attr->st_mode & S_IXOTH ? "x" : "-");
        strcat(entry->buf, " ");
        strcat(entry->buf, pw->pw_name);
        strcat(entry->buf, " ");
        strcat(entry->buf, gr->gr_name);
        strcat(entry->buf, " ");
        sprintf(entry->buf + strlen(entry->buf), listing->metadata->filesize_format, file_attr->st_size);
        strcat(entry->buf, " ");
        strcat(entry->buf, timestamp);
        strcat(entry->buf, " ");
        strcat(entry->buf, dir_ent->d_name);
        strcat(entry->buf, "\n");

        cursor++;
    }

    closedir(dir);

    return listing;
}

char* concat(Listing* listing)
{
    Entry** cursor;
    Entry* entry;
    char* buf;
    int length, offset, i;

    cursor = listing->start;
    offset = 0;

    for (i = 0; i < listing->num_entries; i++)
    {
        entry = *cursor;
        length += entry->length;
        cursor++;
    }

    cursor = listing->start;
    buf = (char *) malloc(sizeof(char) * length);

    for (i = 0; i < listing->num_entries; i++)
    {
        entry = *cursor;
        memcpy(buf + offset, entry->buf, entry->length);
        offset += entry->length;
        cursor++;
    }

    return buf;
}