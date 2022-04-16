#include "includes.h"



int main(int argc, char const *argv[]){
    int task_count;

    if (argc > 2) {
        fprintf(stderr, "Usage: %s <task_count>\n", argv[0]);
        exit(NOT_OK);
    }
    else if (argc == 2) {
        task_count = atoi(argv[1]);
    }
    else if (argc == 1) {
        scanf("%d", &task_count);
    }

    shared_buffer_ADT shared_buffer = open_shared_buffer(SEM_PATH,SHM_PATH, task_count * MAX_BUFF);

    int i = 0;
    char to_print[MAX_BUFF];
    

    while (i <= task_count) {
        shared_buffer_wait(shared_buffer);
        shared_buffer_read(shared_buffer, to_print);

        printf("%s", to_print);

        i++;
    }
    
    unlink_shared_buffer(shared_buffer);

    return OK;
}


