#include "copy.h"

int copy(const char *src, const char *dest) {
    int in, out;

    if ((in = open(src, O_RDONLY)) < 0){
        fprintf(stderr, "Unable to read source file %s", src);
        return -1;
    }
    if ((out = creat(dest, 0660)) < 0) {
        fprintf(stderr, "Unable to read destination file %s", dest);
        close(in);
        return -1;
    }

    off_t bytesCopied = 0;
    struct stat fileinfo = {0};
    if (fstat(in, &fileinfo) < 0) {
        fprintf(stderr, "Unable to read size of file %s", dest);
        close(in);
        close(out);
        return -1;
    }
    int result = sendfile(out, in, &bytesCopied, fileinfo.st_size);

    close(in);
    close(out);

    return result;
}