//*includes----------------------
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/select.h>
#include <limits.h>
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
void send_task(t_communication pipe, const char* task, int* index_tasks);
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

    for (int i = 0; i < num_workers; i++)
    {
        close(pipes[i].pipe_return_answer[1]);
        close(pipes[i].pipe_send_task[0]);
    }
    

    send_initial_tasks(tasks, pipes, &index_task, num_workers);
    
    for (int i = 0; i < num_workers; i++)
    {
        char buff[4096];
        read(pipes[i].pipe_return_answer[0], buff, 4096);
        printf("%s", buff);
    }

    /* while(index_task < num_tasks){

        fd_set read_set;
        FD_ZERO(&read_set);

        fill_set(&read_set, pipes, num_workers);

        if(select(highest_fd_read_answer, &read_set, NULL, NULL, NULL) == -1){
            perror("select error"),
            exit(1);
        }

        for (int i = 0; i < num_workers; i++)
        {
            if(FD_ISSET(pipes[i].pipe_return_answer[0], &read_set) != 0){
                char buff_answer[SSIZE_MAX];
                int length_answer = read(pipes[i].pipe_return_answer[0], buff_answer, SSIZE_MAX);
            }
        }
        
    } */

    for(int i = 0 ; i < num_workers ; i++){
        close(pipes[i].pipe_return_answer[0]);
        close(pipes[i].pipe_send_task[1]);
    }

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
            
            close(pipes[i].pipe_return_answer[0]);
            close(pipes[i].pipe_send_task[1]);

            for (int j = 0; j < num_workers; j++)
            {
                if(j != i){
                    close(pipes[j].pipe_return_answer[0]);
                    close(pipes[j].pipe_return_answer[1]);
                    close(pipes[j].pipe_send_task[0]);
                    close(pipes[j].pipe_send_task[1]);
                }
            }
            
            if(dup2(pipes[i].pipe_return_answer[1], STDOUT_FILENO) == -1){
                perror("dup2 error");
                return;
            }

            if(dup2(pipes[i].pipe_send_task[0], STDIN_FILENO) == -1){
                perror("dup2 error");
                return;
            }

            close(pipes[i].pipe_return_answer[1]);
            close(pipes[i].pipe_send_task[0]);

            printf("hola\n");

            if(execl("./worker", "./worker", NULL)==-1){
                perror("exec failed");
                exit(1);
            }
        }
    }

}

void send_task(t_communication pipe, const char* task, int* index_tasks){

    int len = strlen(task);

    if(write(pipe.pipe_send_task[1], task, len) == -1){
        perror("write error");
    }

    (*index_tasks)++;
}

void send_initial_tasks(const char** tasks, p_communication pipes, int* index_tasks, int num_workers){

    for (int i = 0; i < num_workers; i++)
    {
        send_task(pipes[i], tasks[*index_tasks], index_tasks);
        if (write(pipes[i].pipe_send_task[1], "\n", 1) == -1){
            perror("write");
        }
    }

}