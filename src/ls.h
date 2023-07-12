typedef struct Metadata
{
    int num_entries;
    long long max_file_size;
    int file_size_field_length;
    char file_size_format[10];
} Metadata;

typedef struct DirectoryListing
{
    char* dir_name;
    int status_code;
    char* buf;
    char* error_msg;
    Metadata* metadata;
} DirectoryListing;

void preprocess(DirectoryListing* dl);

DirectoryListing* ls(char* dir_name);