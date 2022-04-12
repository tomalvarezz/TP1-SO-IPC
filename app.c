//*includes----------------------
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
//*------------------------------

//*Defines-----------------------
#define MAX_WORKERS_CAN_CREATE 5
//*------------------------------

typedef struct {
    int pipe_send_task[2];
    int pipe_return_answer[2];
} t_communication;

typedef t_communication *p_communication;

//*Prototypes--------------------
int calculate_workers(int num_tasks);
void create_pipes(p_communication pipes, int num_workers, int* highest_fd_read_answer);
void initialize_workers(const char **tasks, p_communication pipes, int num_workers, int num_tasks, int *index);
void send_initial_tasks(const char** tasks, p_communication pipes, int* index_tasks, int num_workers);
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
    t_communication pipes[num_workers];
    int highest_fd_read_answer=0;
    create_pipes(pipes, num_workers, &highest_fd_read_answer);

    //inicializamos los workers con sus respectivas tareas iniciales
    initialize_workers(tasks, pipes, num_workers, num_tasks, &index_task);

    send_initial_tasks(tasks, pipes, &index_task, num_workers);

    for(int i = 0 ; i < num_workers ; i++){
        wait(NULL);
        close(pipes[i].pipe_return_answer[0]);
        close(pipes[i].pipe_return_answer[1]);
        close(pipes[i].pipe_send_task[0]);
        close(pipes[i].pipe_send_task[1]);
    }

    return 0;
}

int calculate_workers(int num_tasks){

    if(num_tasks < MAX_WORKERS_CAN_CREATE){
        return num_tasks;
    }

    return MAX_WORKERS_CAN_CREATE;
}

void create_pipes(t_communication* pipes, int num_workers, int* highest_fd_read_answer){

    for (int i = 0; i < num_workers; i++)
    {
        if(pipe(pipes[i].pipe_return_answer)== -1){
            perror("pipe failed");
        }
        if(pipe(pipes[i].pipe_send_task)== -1){
            perror("pipe failed");
        }

        if(*highest_fd_read_answer < pipes[i].pipe_return_answer[0]){
            *highest_fd_read_answer = pipes[i].pipe_return_answer[0];
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

            /* if(dup2(pipes[i].pipe_return_answer[1], STDOUT_FILENO) == -1){
                perror("dup2 error");
                return;
            } */

            if(dup2(pipes[i].pipe_send_task[0], STDIN_FILENO) == -1){
                perror("dup2 error");
                return;
            }

            //close(pipes[i].pipe_return_answer[1]);
            close(pipes[i].pipe_send_task[0]);

            if(execl("./worker", "./worker", NULL)==-1){
                perror("exec failed");
                return;
            }
        }
    }

}

void send_task(t_communication pipe, const char* task){

    int len = strlen(task);

    if(write(pipe.pipe_send_task[1], task, len) == -1){
        perror("write error");
    }
}

void send_initial_tasks(const char** tasks, p_communication pipes, int* index_tasks, int num_workers){

    for (int i = 0; i < num_workers; i++)
    {
        send_task(pipes[i], tasks[*index_tasks]);
        (*index_tasks)++;
    }

}