/*/////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                               //
//                                                         ___________________________.          //  
//                                                        |;;|                     |;;||         //  
//  Group: Six-One(-2)                                    |[]|---------------------|[]||         //  
//  Authors:                                              |;;|    |\ /`\ / |/      |;;||         //  
//      Cody Vernon                                       |;;|    |< |-| | |       |;;||         //  
//      Jack Harvison                                     |;;|    |/ | | \ |\      |;;||         //  
//      Drake Farmer                                      |;;|    ___   | | |\     |;;||         //  
//                                                        |;;|  |  |    | | |/     |;;||         //              
//  Assignment: BackItUp file backup system               |;;|  |  |    |_| |      |;;||         //  
//  Course: CS 460 Operating Systems                      |;;|_____________________|;;||         //  
//                                                        |;;;;;;;;;;;;;;;;;;;;;;;;;;;||         //  
//                                                        |;;;;;;_______________ ;;;;;||         //  
//                                                        |;;;;;|  ___          |;;;;;||         //  
//                                                        |;;;;;| |;;;|         |;;;;;||         //  
//                                                        |;;;;;| |;;;|         |;;;;;||         //  
//                                                        |;;;;;| |;;;|         |;;;;;||         //  
//                                                        |;;;;;| |;;;|         |;;;;;||         //  
//                                                        |;;;;;| |___|         |;;;;;||         //  
//                                                        \_____|_______________|_____||         //  
//                                                         ~~~~~^^^^^^^^^^^^^^^^^~~~~~~          //      
//                                                                                               //  
/////////////////////////////////////////////////////////////////////////////////////////////////*/

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

//Dir_info_t structure
//Used to hold information about a directory
typedef struct {
    char *path;
    int num_files;
    int num_dirs;
    char **file_list;
    char **dir_list;
    int has_backup_dir; 
} Dir_info_t;

// parse command line arguments
void parseArgs(int argc, char const *argv[], int *operation);
void runBackup(char *path);
void runRestore(char *path);
Dir_info_t *getDirInfo(char *path);
void free_list(char **list, int size);

int main(int argc, char const *argv[]){

    int operation = 0; // 1 for restore, 0 for backup

    parseArgs(argc, argv, &operation);

    // get current directory path
    char dir_path[PATH_MAX];
    if (getcwd(dir_path, PATH_MAX) == NULL) {
        perror("getcwd() failed");
        exit(1);
    }

    //If we're in debug state, let the user know the operation and directory path
    if (DEBUG) printf("Operation %d, Directory Path: %s\n", operation, dir_path);

    //run the appropriate operation, 0 = backup and 1 = restore
    if (operation == 0) {
        runBackup(dir_path);
    } else {
        runRestore(dir_path);
    }

    return 0;
}

//parseArgs
//This function takes in argc, argv, and operation. It reads in the arguments from the command line and sets the operation accordingly
void parseArgs(int argc, char const *argv[], int *operation){
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0) *operation = 1;
    }
}

//runBackup
//This function will back up all files to their same layer .backup folder
//If no .backup folder exists it will create one
void runBackup(char *path){
    if (DEBUG) printf("\nBackup: %s\n", path);

    // get directory information
    Dir_info_t *dir_info = getDirInfo(path);

    // if no backup directory was found, create it
    if (!dir_info->has_backup_dir) {
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
    for (int i = 0; i < dir_info->num_dirs; i++) {
        if (DEBUG) printf("%s [directory]\n", dir_info->dir_list[i]);
        // recursively go down another folder
        char new_path[PATH_MAX];
        sprintf(new_path, "%s/%s", path, dir_info->dir_list[i]);
        runBackup(new_path);//For each layer of directories, execute runBackup at this layer
    }
    //Free memory
    free_list(dir_info->dir_list, dir_info->num_dirs);

    pthread_t threads[dir_info->num_files];
    Copy_args_t copy_args[dir_info->num_files];
    // iterate through files
    for (int i = 0; i < dir_info->num_files; i++) {
        if (DEBUG) printf("%s/%s [file]\n", path, dir_info->file_list[i]);
        // spawn off a thread for the file
        copy_args[i].path = path;
        copy_args[i].file_name = dir_info->file_list[i];
        //Spin up a thread
        if (pthread_create(&threads[i], NULL, backup, &copy_args[i]) != 0 ) {
            perror("Error creating thread");
        }
    }

    // join the threads
    for (int i = 0; i < dir_info->num_files; i++) {
        pthread_join(threads[i], NULL);
    }
    //Free memory
    free_list(dir_info->file_list, dir_info->num_files);

    // base case when there are no more folders 
    if (dir_info->num_dirs == 0) {
        free(dir_info);
        return;
    }

    //Free memory
    free(dir_info);
}


//runRestore
//This function will overwrite all applicable files to their most recent backed up version
void runRestore(char *path){
    char new_bak_path[PATH_MAX];
    sprintf(new_bak_path, "%s/%s", path, ".backup");

    if (DEBUG) printf("Restore: %s\n", new_bak_path);

    Dir_info_t *dir_info = getDirInfo(path);

    // iterate through directories
    for (int i = 0; i < dir_info->num_dirs; i++) {
        if (DEBUG) printf("%s [directory]\n", dir_info->dir_list[i]);
        // recursively go down another folder
        char new_path[PATH_MAX];
        sprintf(new_path, "%s/%s", path, dir_info->dir_list[i]);
        runRestore(new_path);
    }
    //Free memory
    free_list(dir_info->dir_list, dir_info->num_dirs);
    free_list(dir_info->file_list, dir_info->num_files);

    // if there is not a backup directory
    if (!dir_info->has_backup_dir) {
        free(dir_info);
        return;
    }

    // get info on backup directory that exists
    Dir_info_t *backup_info = getDirInfo(new_bak_path);

    pthread_t threads[backup_info->num_files];
    Copy_args_t copy_args[backup_info->num_files];
    // iterate through files in backup
    for (int i = 0; i < backup_info->num_files; i++) {
        if (DEBUG) printf("%s/%s [file]\n", dir_info->path, backup_info->file_list[i]);
        // spawn off a thread for the file
        copy_args[i].path = dir_info->path;
        copy_args[i].file_name = backup_info->file_list[i];
        //Spin up a new thread
        if (pthread_create(&threads[i], NULL, restore, &copy_args[i]) != 0 ) {
            perror("Error creating thread");
        }
    }
    //Free memory
    free_list(backup_info->dir_list, backup_info->num_dirs);

    // join the threads
    for (int i = 0; i < backup_info->num_files; i++) {
        pthread_join(threads[i], NULL);
    }
    free_list(backup_info->file_list, backup_info->num_files);


    // base case when there are no more folders 
    if (dir_info->num_dirs == 0) {
        free(dir_info);
        free(backup_info);
        return;
    }

    //Free memory
    free(backup_info);
    free(dir_info);
}

//getDirInfo
//This function collects all information in a directory and returns a Dir_info_t struct with the results
Dir_info_t *getDirInfo(char *path) {
    DIR *dir;
    struct dirent *entry = NULL;
    int has_backup_dir = 0;
    int num_files = 0;
    int num_dirs = 0;
    char **dir_list = NULL;
    char **file_list = NULL;

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

            // increment number of files and add file name onto dir_list
            num_files++;
            file_list = realloc(file_list, num_files * sizeof(char *));
            file_list[num_files - 1] = strdup(entry->d_name);
        }
    }
    closedir(dir);

    //Fill the struct with our data
    Dir_info_t *return_info = malloc(sizeof(Dir_info_t));
    return_info->path = path; 
    return_info->num_files = num_files;
    return_info->num_dirs = num_dirs;
    return_info->file_list = file_list;
    return_info->dir_list = dir_list;
    return_info->has_backup_dir = has_backup_dir;
    return return_info; //Return the struct
}


//free_list
//This function frees all items within a given list
void free_list(char **list, int size){
    for (int i = 0; i < size; i++) {
        free(list[i]);
    }
    free(list);
}