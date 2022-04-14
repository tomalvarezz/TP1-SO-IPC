#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

void solve_task(char *task);

int main(int argc, char *argv[])
{

    if (setvbuf(stdout, NULL, _IONBF, 0) == -1)
    {
        perror("setvbuf error");
    }

    char *task = NULL;
    size_t len = 0;
    ssize_t nread;

    while ((nread = getline(&task, &len, stdin)) != 0)
    {
        // task[strcspn(task, "\n")]= 0;
        // fprintf(stderr, "worker: %s\n", task);
        solve_task(task);
    }

    free(task);
    exit(EXIT_SUCCESS);
}

void solve_task(char *task)
{

    char solved_task[4096];
    int length;

    /* FILE* result_task;
    if(result_task = popen(solved_task, "r") == -1){
        perror("popen error");
    } */

    printf("%s", task);
}