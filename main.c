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
// return 0 for regular backup, 1 for restore
int parseArgs(int argc, char const *argv[]);

void runBackup();

void runRestore();

int main(int argc, char const *argv[]){
    if (parseArgs(argc, argv) == 0) {
        runBackup();
    } else {
        runRestore();
    }
    return 0;
}

int parseArgs(int argc, char const *argv[]){
    if (argc > 1 && strcmp(argv[1], "-r") == 0) return 1;
    else if (argc > 1 && strcmp(argv[1], "-r") != 0) {
        fprintf(stderr, "Invalid option '%s'\n", argv[1]);
        exit(1);
    }
    return 0;
}

void runBackup(){
    if (DEBUG) printf("Backup\n");
}

void runRestore(){
    if (DEBUG) printf("Restore\n");
}