#include "copy.h"

int copy(const char *src, const char *dest) {
    int in, out;
    // Open source file
    if ((in = open(src, O_RDONLY)) < 0){
        fprintf(stderr, "Unable to read source file %s\n", src);
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
        close(in);
        close(out);
        return -1;
    }

    int result = sendfile(out, in, &bytesCopied, fileinfo.st_size);
    if (result >= 0) {
        printf("Copied %ld bytes from %s to %s\n", fileinfo.st_size, src, dest);
    }

    close(in);
    close(out);

    return result;
}



void *backup(void *args){

    Copy_args_t *file = (Copy_args_t *)args;

    char file_path[PATH_MAX];
    char backup_path[PATH_MAX];
    char backup_file_name[PATH_MAX];

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
            printf("WARNING: Overwriting %s\n", backup_file_name);
            remove(dest);
            if (copy(src, dest) < 0) {
                fprintf(stderr, "unable to copy %s into backup %s\n", src, dest);
            } 
        } else { 
            printf("%s does not need backing up\n", backup_file_name);
        }

    } else {
        
        if (copy(src, dest) < 0) {
            fprintf(stderr, "unable to copy %s into backup %s\n", src, dest);
        } 
    }
    return NULL;
}
void *restore(void *args) {

    Copy_args_t *file = (Copy_args_t *)args;

    char file_path[PATH_MAX];
    char file_name[PATH_MAX];
    char backup_path[PATH_MAX];
    
    // Get destination path
    size_t path_len = strlen(file->path);
    strncpy(file_path, file->path, path_len - 8);
    
    size_t name_len = strlen(file->file_name);
    strncpy(file_name, file->file_name, name_len - 4);
    
    strcat(file_path, "/");
    char *dest = strcat(file_path, file_name);

    // Get source path
    strcpy(backup_path, file->path);
    strcat(backup_path, "/");
    char *src = strcat(backup_path, file->file_name);

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
            printf("WARNING: Overwriting %s", file_name);
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