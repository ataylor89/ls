#ifndef LS_H
#define LS_H

#include "types.h"

DirectoryList* get_directory_list(char* dirname);
void sort_directory_list(DirectoryList* dl);
void format_directory_list(DirectoryList* dl);

#endif