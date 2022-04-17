// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "includes.h"

typedef struct {
    int pipe_send_task[2];
    int pipe_return_answer[2];
} t_communication;

typedef t_communication *p_communication;

//*Prototypes--------------------
int calculate_workers(int num_tasks);
void create_pipes(p_communication pipes, int num_workers, int* highest_fd_read_answer);
void initialize_workers(const char **tasks, p_communication pipes, int num_workers, int num_tasks, int *index);
void send_initial_tasks(const char** tasks, p_communication pipes, int* index_tasks, int num_workers, int tasks_per_worker);
void send_task(t_communication pipe, const char** tasks, int* index_tasks);
int calculate_tasks_per_worker(int num_workers, int num_tasks);
void fill_set(fd_set* read_set, p_communication pipes, int num_workers);
//*------------------------------

int main(int argc, char const* argv[]){
    if(argc < 2){
        fprintf(stderr, "Usage: %s <files>\n", argv[0]);
        return 1;
    }

    int num_tasks = argc-1;
    const char** tasks = argv+1;
    int index_task = 0;
    int solved_tasks = 0;
    int num_workers = calculate_workers(num_tasks);

    shared_buffer_ADT shared_buffer = create_shared_buffer(SEM_PATH, SHM_PATH, num_tasks*MAX_BUFF);

    FILE * results_file;
    results_file = fopen("results.txt", "w+");
    if (results_file == NULL){
        perror("fopen");
        exit(NOT_OK);
    }

    sleep(2);
    printf("%d", num_tasks);

    int tasks_per_worker = calculate_tasks_per_worker(num_workers, num_tasks);

    t_communication pipes[num_workers];
    int highest_fd_read_answer = 0;
    create_pipes(pipes, num_workers, &highest_fd_read_answer);

    initialize_workers(tasks, pipes, num_workers, num_tasks, &index_task);

    for (int i = 0; i < num_workers; i++){
        close(pipes[i].pipe_return_answer[1]);
        close(pipes[i].pipe_send_task[0]);
    }
    
    send_initial_tasks(tasks, pipes, &index_task, num_workers, tasks_per_worker);

    while(solved_tasks < num_tasks){
        char buff_answer[MAX_BUFF] = {0};
        fd_set read_set;
        FD_ZERO(&read_set);
        fill_set(&read_set, pipes, num_workers);

        if (select(highest_fd_read_answer + 1, &read_set, NULL, NULL, NULL) == ERROR){
            perror("select error"),
            exit(NOT_OK);
        }

        for (int i = 0; i < num_workers; i++){
            if(FD_ISSET(pipes[i].pipe_return_answer[0], &read_set) != 0){
                if ((read(pipes[i].pipe_return_answer[0], buff_answer, MAX_BUFF)) == ERROR){
                    perror("read error");
                    exit(NOT_OK);
                }

                char * answer = strtok(buff_answer, "\n");

                while(answer != NULL){
                    shared_buffer_send(shared_buffer, answer);

                    shared_buffer_post(shared_buffer);

                    if(fprintf(results_file, "%s\n", answer)<0){
                        perror("fprintf");
                        exit(NOT_OK);
                    }

                    answer = strtok(NULL, "\n");
                    solved_tasks++;
                }

                if(index_task < num_tasks){
                    send_task(pipes[i], tasks, &index_task); 
                }
                
            }
        }
    }

    for(int i = 0 ; i < num_workers ; i++){
        close(pipes[i].pipe_return_answer[0]);
        close(pipes[i].pipe_send_task[1]);
    }

    for(int i = 0 ; i < num_workers ; i++){
        wait(NULL);
    }

    if(fclose(results_file) == EOF) {
        perror("fclose");
        exit(NOT_OK);
    }

    close_shared_buffer(shared_buffer);

    return OK;
}

int calculate_workers(int num_tasks){
    if(num_tasks < MAX_WORKERS_CAN_CREATE){
        return num_tasks;
    }
    return MAX_WORKERS_CAN_CREATE;
}

int calculate_tasks_per_worker(int num_workers, int num_tasks){
    if(num_workers*MAX_TASKS_PER_WORKER < num_tasks){
        return MAX_TASKS_PER_WORKER;
    }
    return 1;
}

void create_pipes(p_communication pipes, int num_workers, int* highest_fd_read_answer){
    for (int i = 0; i < num_workers; i++){
        if (pipe(pipes[i].pipe_return_answer) == ERROR){
            perror("pipe failed");
        }

        if (pipe(pipes[i].pipe_send_task) == ERROR){
            perror("pipe failed");
        }

        if(*highest_fd_read_answer < pipes[i].pipe_return_answer[0]){
            *highest_fd_read_answer = pipes[i].pipe_return_answer[0];
        }
    }
}

void initialize_workers(const char** tasks, p_communication pipes, int num_workers, int num_tasks, int* index_task){
    for (int i = 0; i < num_workers; i++){
        int pid;
        if ((pid = fork()) == ERROR){
            perror("fork failed");
            exit(NOT_OK);
        }

        if(pid == 0){
            close(pipes[i].pipe_return_answer[0]);
            close(pipes[i].pipe_send_task[1]);

            for (int j = 0; j < num_workers; j++){
                if(j != i){
                    close(pipes[j].pipe_return_answer[0]);
                    close(pipes[j].pipe_return_answer[1]);
                    close(pipes[j].pipe_send_task[0]);
                    close(pipes[j].pipe_send_task[1]);
                }
            }

            if (dup2(pipes[i].pipe_return_answer[1], STDOUT_FILENO) == ERROR){
                perror("dup2 error");
                exit(NOT_OK);
            }

            if (dup2(pipes[i].pipe_send_task[0], STDIN_FILENO) == ERROR){
                perror("dup2 error");
                exit(NOT_OK);
            }

            close(pipes[i].pipe_return_answer[1]);
            close(pipes[i].pipe_send_task[0]);

            if (execl("./worker", "./worker", NULL) == ERROR){
                perror("exec failed");
                exit(NOT_OK);
            }
        }
    }
}

void send_task(t_communication pipe, const char** tasks, int* index_tasks){
    int len = strlen(tasks[*index_tasks]);
    if (write(pipe.pipe_send_task[1], tasks[*index_tasks], len) == ERROR){
        perror("write error");
        exit(NOT_OK);
    }
    if (write(pipe.pipe_send_task[1], "\n", 1) == ERROR){
        perror("write");
        exit(NOT_OK);
    }

    (*index_tasks)++;
}

void send_initial_tasks(const char** tasks, p_communication pipes, int* index_tasks, int num_workers, int num_tasks_per_worker){
    for (int i = 0; i < num_workers; i++){
        for (int j = 0; j < num_tasks_per_worker ; j++){
            send_task(pipes[i], tasks, index_tasks);
        }
    }
}

void fill_set(fd_set* read_set, p_communication pipes, int num_workers){
    for (int i = 0; i < num_workers; i++){
        FD_SET(pipes[i].pipe_return_answer[0], read_set);
    }
}