#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "options.h"

// This file needs to recursively go down through the folders in the current folder
// create .backup folder in current directory
// spawn a new thread for every file in the directory to copy each one into .backup
// go down a folder

// parse command line arguments
void parseArgs(int argc, char const *argv[], int *operation, char const **src_path);

void runBackup(char const *src_path);

void runRestore(char const *src_path);

int main(int argc, char const *argv[]){

    char const *src_path = NULL; // path to directory to backup or restore
    int operation = 0; // 1 for restore, 0 for backup

    parseArgs(argc, argv, &operation, &src_path);

    if (src_path == NULL) {
        fprintf(stderr, "Error: Directory path not specified\n");
        exit(1);
    }

    if (DEBUG) printf("Operation %d, Directory Path: %s\n", operation, src_path);

    if (operation == 0) {
        runBackup(src_path);
    } else {
        runRestore(src_path);
    }

    return 0;
}

void parseArgs(int argc, char const *argv[], int *operation, char const **src_path){
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0) *operation = 1;
        else *src_path = argv[i];
    }
}

void runBackup(char const *src_path){
    if (DEBUG) printf("Backup: %s\n", src_path);
}

void runRestore(char const *src_path){
    if (DEBUG) printf("Restore: %s\n", src_path);
}