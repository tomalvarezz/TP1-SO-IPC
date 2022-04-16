#include "shared_buffer_ADT.h"
#include <stdlib.h>

int main(int argc, char const *argv[]){
    int task_count;

    if (argc > 2) {
        fprintf(stderr, "Usage: %s <task_count>\n", argv[0]);
        exit(1);
    }
    else if (argc == 2) {
        task_count = atoi(argv[1]);
    }
    else if (argc == 1) {
        scanf("%d", &task_count);
    }

    shared_buffer_ADT shared_buffer = open_shared_buffer(SEM_PATH,SHM_PATH, task_count * 4096);

    int i = 0;
    char to_print[4096];
    int length;

    while (i <= task_count) {
        shared_buffer_wait(shared_buffer);
        length=shared_buffer_read(shared_buffer, to_print);

        printf("%s", to_print);

        i++;
    }
    
    unlink_shared_buffer(shared_buffer);

    return 0;
}


