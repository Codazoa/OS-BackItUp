#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include "options.h"
#include "copy.h"

// This file needs to recursively go down through the folders in the current folder
// create .backup folder in current directory
// spawn a new thread for every file in the directory to copy each one into .backup
// go down a folder

// parse command line arguments
void parseArgs(int argc, char const *argv[], int *operation);

void runBackup(char *path);

void runRestore(char *path);

int main(int argc, char const *argv[]){

    int operation = 0; // 1 for restore, 0 for backup

    parseArgs(argc, argv, &operation);

    // get current directory path
    char dir_path[PATH_MAX];
    if (getcwd(dir_path, PATH_MAX) == NULL) {
        perror("getcwd() failed");
        exit(1);
    }

    if (DEBUG) printf("Operation %d, Directory Path: %s\n", operation, dir_path);

    if (operation == 0) {
        runBackup(dir_path);
    } else {
        runRestore(dir_path);
    }

    return 0;
}

void parseArgs(int argc, char const *argv[], int *operation){
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0) *operation = 1;
    }
}

void runBackup(char *path){
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    int has_backup_dir = 0;
    int num_files = 0;
    int num_dirs = 0;
    char **dir_list = NULL;
    char **file_list = NULL;

    if (DEBUG) printf("\nBackup: %s\n", path);

    // open this directory
    dir = opendir(path);
    if (dir == NULL) {
        perror("backitup Error");
        exit(1);
    }

    // analyze the directory information
    // check if .backup exists
    // add directories to dir_list
    // add files to file_list
    while ((entry = readdir(dir)) != NULL) {
        // sprintf(name, "%s", entry->d_name);
        if (entry->d_type == DT_DIR) {
            // is a directory

            // don't add the . or .. directories to the list
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            // does the current directory have a .backup folder
            if (strcmp(entry->d_name, ".backup") == 0) {
                has_backup_dir = 1;
            } else {
                // increment number of directories and add directory name onto dir_list
                num_dirs++;
                dir_list = realloc(dir_list, num_dirs * sizeof(char *));
                dir_list[num_dirs - 1] = strdup(entry->d_name);
            }

        } else {
            // is a file
            if (DEBUG) printf("%s\n", entry->d_name);

            // increment number of files and add file name onto file_list
            num_files++;
            file_list = realloc(file_list, num_files * sizeof(char *));
            file_list[num_files - 1] = strdup(entry->d_name);
        }
    }
    closedir(dir);

    // if no backup directory was found, create it
    if (!has_backup_dir) {
        mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;
        char backup_path[PATH_MAX];
        sprintf(backup_path, "%s/%s", path, ".backup");
        int result = mkdir(backup_path, mode);

        if (result == 0){
            printf(".backup created successfully.\n");
        } else {
            perror("Error: Unable to create .backup\n");
        }
    }

    // iterate through directories
    for (int i = 0; i < num_dirs; i++) {
        if (DEBUG) printf("%s [directory]\n", dir_list[i]);
        // recursively go down another folder
        char new_path[PATH_MAX];
        sprintf(new_path, "%s/%s", path, dir_list[i]);
        runBackup(new_path);
    }
    free(dir_list);

    pthread_t threads[num_files];
    Copy_args_t copy_args[num_files];
    // iterate through files
    for (int i = 0; i < num_files; i++) {
        if (DEBUG) printf("%s/%s [file]\n", path, file_list[i]);
        // spawn off a thread for the file
        copy_args[i].path = path;
        copy_args[i].file_name = file_list[i];
        if (pthread_create(&threads[i], NULL, backup, &copy_args[i]) != 0 ) {
            perror("Error creating thread");
        }
    }

    // join the threads
    for (int i = 0; i < num_files; i++) {
        pthread_join(threads[i], NULL);
    }
    free(file_list);

    // base case when there are no more folders 
    if (num_dirs == 0) {
        return;
    }
}

void runRestore(char *path){
    // struct dirent *entry = readdir(dir);
    // if (DEBUG) printf("Restore: %s\n", entry->d_name);
}