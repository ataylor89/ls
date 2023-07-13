typedef struct Entry
{
    char* filename;
    char* buf;
    int length;
} Entry;

typedef struct Metadata
{
    long long max_filesize;
    int filesize_field_length;
    char filesize_format[10];
} Metadata;

typedef struct Listing
{
    char* dirname;
    int num_entries;
    Entry** start;
    char* buf;
    int length;
    Metadata* metadata;
    int status_code;
    char* error_msg;
} Listing;

Listing* ls(char* dirname);
void concat(Listing* listing);