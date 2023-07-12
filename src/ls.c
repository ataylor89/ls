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

int num_digits(long long file_size)
{
    int count = 0;

    while (file_size > 0)
    {
        count++;
        file_size /= 10;
    }

    return count;
}

void preprocess(DirectoryListing* dl)
{
    Metadata* metadata;
    DIR* dir;
    struct dirent* dir_ent;
    struct stat* file_attr;
    char file_name[200];

    dl->metadata = (Metadata *) malloc(sizeof(Metadata));
    metadata = dl->metadata;
    metadata->num_entries = 0;
    metadata->max_file_size = 0;
    file_attr = (struct stat *) malloc(sizeof(struct stat));

    if ((dir = opendir(dl->dir_name)) == NULL)
    {
        dl->status_code = 1;
        dl->error_msg = "Error opening directory.\n";
        return;
    }

    while ((dir_ent = readdir(dir)) != NULL)
    {
        sprintf(file_name, "%s/%s", dl->dir_name, dir_ent->d_name);

        if (stat(file_name, file_attr) == -1)
        {
            dl->status_code = 1;
            sprintf(dl->error_msg, "Error calling stat on file %s\n", file_name);
            return;
        }

        if (file_attr->st_size > metadata->max_file_size)
        {
            metadata->max_file_size = file_attr->st_size;
        }

        metadata->num_entries++;
    }

    metadata->file_size_field_length = num_digits(metadata->max_file_size);
    sprintf(metadata->file_size_format, "%%%dlld", metadata->file_size_field_length);

    if (closedir(dir) != 0)
    {
        dl->status_code = 1;
        dl->error_msg = "Error closing directory\n";
    }
}

DirectoryListing* ls(char* dir_name)
{
    DirectoryListing* dl;
    DIR* dir;
    struct dirent* dir_ent;
    struct stat* file_attr;
    char file_name[200];
    struct passwd* pw;
    struct group* gr;

    dl = (DirectoryListing *) malloc(sizeof(DirectoryListing));
    dl->dir_name = dir_name;
    dl->buf = (char *) malloc(10000);
    dl->error_msg = (char *) malloc(100);
    file_attr = (struct stat *) malloc(sizeof(struct stat));

    preprocess(dl);

    if (dl->status_code != 0)
    {
        return dl;
    }

    if ((dir = opendir(dir_name)) == NULL)
    {
        dl->status_code = 1;
        dl->error_msg = "Error opening directory.\n";
        return dl;
    }

    while ((dir_ent = readdir(dir)) != NULL)
    {
        sprintf(file_name, "%s/%s", dir_name, dir_ent->d_name);

        if (stat(file_name, file_attr) == -1)
        {
            dl->status_code = 1;
            sprintf(dl->error_msg, "Error calling stat on file %s\n", file_name);
            return dl;
        }

        pw = getpwuid(file_attr->st_uid);
        gr = getgrgid(file_attr->st_gid);

        strcat(dl->buf, dir_ent->d_type == DT_DIR ? "d" : "-");
        strcat(dl->buf, file_attr->st_mode & S_IRUSR ? "r" : "-");
        strcat(dl->buf, file_attr->st_mode & S_IWUSR ? "w" : "-");
        strcat(dl->buf, file_attr->st_mode & S_IXUSR ? "x" : "-");
        strcat(dl->buf, file_attr->st_mode & S_IRGRP ? "r" : "-");
        strcat(dl->buf, file_attr->st_mode & S_IWGRP ? "w" : "-");
        strcat(dl->buf, file_attr->st_mode & S_IXGRP ? "x" : "-");
        strcat(dl->buf, file_attr->st_mode & S_IROTH ? "r" : "-");
        strcat(dl->buf, file_attr->st_mode & S_IWOTH ? "w" : "-");
        strcat(dl->buf, file_attr->st_mode & S_IXOTH ? "x" : "-");
        strcat(dl->buf, " ");
        strcat(dl->buf, pw->pw_name);
        strcat(dl->buf, " ");
        strcat(dl->buf, gr->gr_name);
        strcat(dl->buf, " ");
        sprintf(dl->buf + strlen(dl->buf), dl->metadata->file_size_format, file_attr->st_size);
        strcat(dl->buf, " ");
        strftime(dl->buf + strlen(dl->buf), 20, "%b %d %H:%M", localtime(&file_attr->st_mtime));
        strcat(dl->buf, " ");
        strcat(dl->buf, dir_ent->d_name);
        strcat(dl->buf, "\n");
    }

    closedir(dir);

    return dl;
}