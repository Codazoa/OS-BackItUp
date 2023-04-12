#ifndef COPY_H
#define COPY_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
// This file needs to take a file and copy it to the existing .backup folder
// in the current directory

typedef struct args {
    char *path;
    char *file_name;
} copy_args;

int copy(const char *src, const char *dest);

void *backup(void *src, void *dest);
void *restore(void *src, void *dest);


#endif
