#include "copy.h"

//copy
//This function takes a source file and copies the contents into a destination file
int copy(const char *src, const char *dest) {
    int in, out;
    // Open source file
    if ((in = open(src, O_RDONLY)) < 0){
        return -1;
    }

    // if destination file exists, open it. If not, then create it
    if (access(dest, F_OK) != -1) {
        if ((out = open(dest, 0660)) < 0){
                fprintf(stderr, "Unable to open dest file %s for writing\n", dest);
                return -1;
            }

    } else {
        if ((out = creat(dest, 0660)) < 0) {
            fprintf(stderr, "Unable to create destination file %s\n", dest);
            close(in);
            return -1;
        }
    }


    // get size of source file and send to destination
    off_t bytesCopied = 0;
    struct stat fileinfo = {0};
    if (fstat(in, &fileinfo) < 0) {
        fprintf(stderr, "Unable to read size of file %s\n", dest);
        close(out);
        close(in);
        return -1;
    }

    int result = sendfile(out, in, &bytesCopied, fileinfo.st_size);

    close(out);
    close(in);

    // update src file access time
    char touch[PATH_MAX + 6];
    sprintf(touch, "touch \"%s\"", src);
    system(touch);

    return result;
}

//backup
//This function saves files into the appropriate backup directory if it exists
void *backup(void *args) {

    Copy_args_t *file = (Copy_args_t *)args;

    char file_path[PATH_MAX];
    char backup_path[PATH_MAX];
    char backup_file_name[PATH_MAX];

    //Adjust path accordingly
    strcpy(file_path, file->path);
    strcat(file_path, "/");
    char *src = strcat(file_path, file->file_name);

    strcpy(backup_path, file->path);
    strcat(backup_path, "/.backup/");

    strcpy(backup_file_name, file->file_name);
    strcat(backup_file_name, ".bak");

    char *dest = strcat(backup_path, backup_file_name);

    struct stat src_stat, dest_stat;
    printf("Backing up %s\n", file->file_name);

    // if destination already exists, only copy if source is more recent
    if (access(dest, F_OK) != -1) {

        if (stat(src, &src_stat) != 0) {
            fprintf(stderr, "Unable to find stats for %s\n", src);
            exit(1);
        }
        if (stat(dest, &dest_stat) != 0) {
            fprintf(stderr, "Unable to find stats for %s\n", dest);
            exit(1);
        }
        
        if (src_stat.st_mtime > dest_stat.st_mtime) {
            printf("WARNING: Overwriting %s\n", backup_file_name);  //Warn the user we are overwriting a file
            remove(dest);                                           //Delete the destination file, then copy over the new file
            if (copy(src, dest) < 0) {
                fprintf(stderr, "Unable to copy %s into backup %s.bak\n", file->file_name, file->file_name);
            } 
        } else { 
            printf("%s does not need backing up\n", backup_file_name);
        }

    } else {
        
        if (copy(src, dest) < 0) {
            fprintf(stderr, "Unable to copy %s into backup %s.bak\n", file->file_name, file->file_name);
        } 
    }
    return NULL;
}

//restore
//This function will overwrite a file with its corresponding version in the backup directory
void *restore(void *args) {

    Copy_args_t *file = (Copy_args_t *)args;

    char dest[PATH_MAX];
    char file_name[PATH_MAX];
    char src[PATH_MAX];
    
    // Create destination path
    strcpy(file_name, file->file_name);
    remove_bak_extension(file_name);
    strcpy(dest, file->path);
    strcat(dest, "/");
    strcat(dest, file_name);

    // Create source path (backup_path)
    sprintf(src, "%s/%s/%s", file->path, ".backup", file->file_name);

    struct stat src_stat, dest_stat;
    printf("Restoring %s\n", file_name);
    
    // if destination already exists, only copy if source is more recent
    if (access(dest, F_OK) != -1) {

        if (stat(src, &src_stat) != 0) {
            fprintf(stderr, "Unable to find stats for %s\n", src);
            exit(1);
        }
        if (stat(dest, &dest_stat) != 0) {
            fprintf(stderr, "Unable to find stats for %s\n", dest);
            exit(1);
        }
        
        if (src_stat.st_mtime > dest_stat.st_mtime) {
            printf("WARNING: Overwriting %s\n", file_name);
            if (copy(src, dest) < 0) {
                fprintf(stderr, "unable to copy %s from backup %s\n", dest, src);
            } 
        } else { 
            printf("%s is already the most current version\n", file_name);
        }

    } else {
        
        if (copy(src, dest) < 0) {
            fprintf(stderr, "unable to copy %s from backup %s\n", dest, src);
        } 
    }
    return NULL;
}

//remove_bak_extension
//This function will clean up files after we are done and remove the .bak extension
void remove_bak_extension(char *filename) {
    int len = strlen(filename);
    // step backwards through the filename
    for (int i = len - 1; i >= 0; i--) {
        // at a . check if remaining words are bak
        if (filename[i] == '.' && strcmp(&filename[i + 1], "bak") == 0) {
            // null terminate string on .
            filename[i] = '\0';
            return;
        }
    }
}