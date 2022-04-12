//*includes----------------------
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
//*------------------------------

//*Defines-----------------------
#define MAX_WORKERS_CAN_CREATE 5
//*------------------------------

typedef struct {
    int pipe_send_task[2];
    int pipe_return_answer[2];
} t_communication;

//*Prototypes--------------------
int calculate_workers(int num_tasks);
void create_pipes(t_communication* pipes, int num_workers);
void initialize_workers(const char** tasks, t_communication* pipes, int num_workers, int num_tasks, int* index);
//*------------------------------

int main(int argc, char const* argv[]){

    //nos aseguramos que nos pasen al menos una tarea
    if(argc < 2){
        fprintf(stderr, "Usage: %s <files>\n", argv[0]);
        return 1;
    }

    int num_tasks = argc-1;
    const char** tasks = argv+1;
    int index_task = 0;
    int num_workers = calculate_workers(num_tasks);

    //inicializamos los pipes antes de forkear
    t_communication* pipes;
    create_pipes(pipes, num_workers);

    //inicializamos los workers con sus respectivas tareas iniciales
    initialize_workers(tasks, pipes, num_workers, num_tasks, &index_task);

    for(int i = 0 ; i < num_workers ; i++){
        wait(NULL);
    }

    return 0;
}

int calculate_workers(int num_tasks){

    if(num_tasks < MAX_WORKERS_CAN_CREATE){
        return num_tasks;
    }

    return MAX_WORKERS_CAN_CREATE;
}

void create_pipes(t_communication* pipes, int num_workers){

    for (int i = 0; i < num_workers; i++)
    {
        if(pipe(pipes[i].pipe_return_answer)== -1){
            perror("pipe failed");
        }
        if(pipe(pipes[i].pipe_send_task)== -1){
            perror("pipe failed");
        }
    }

}

void initialize_workers(const char** tasks, t_communication* pipes, int num_workers, int num_tasks, int* index_task){


    for (int i = 0; i < num_workers; i++)
    {
        int pid;
        if((pid = fork()) == -1){
            perror("fork failed");
            return;
        }

        if(pid == 0){
            if(execl("./worker", "./worker", NULL)==-1){
                perror("exec failed");
                return;
            }
        }
    }

}