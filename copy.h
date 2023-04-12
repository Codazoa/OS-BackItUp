#ifndef COPY_H
#define COPY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
// This file needs to take a file and copy it to the existing .backup folder
// in the current directory

typedef struct args {
    char *path; // path to directory of the file
    char *file_name; // name of file
} Copy_args_t;

int copy(const char *src, const char *dest);

void *backup(void *args);
void *restore(void *args);


#endif
