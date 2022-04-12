#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

int main(int argc, char *argv[]){
    if(setvbuf(stdout, NULL, _IONBF, 0) == -1){perror("setvbuf error");}

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    /* while ((nread = getline(&line, &len, stdin)) != -1) {
        fwrite(line, nread, 1, stderr);
    } */

    nread = getline(&line, &len, stdin);
    fwrite(line, nread, 1, stderr);

    free(line);
    exit(EXIT_SUCCESS);
} 