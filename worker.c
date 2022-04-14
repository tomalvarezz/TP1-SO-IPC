#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

void solve_task(char* task_path);

int main(int argc, char *argv[]){

    if(setvbuf(stdout, NULL, _IONBF, 0) == -1){perror("setvbuf error");}
    
    char *task_path = NULL;
    size_t len = 0;
    ssize_t nread;

    while ((nread = getline(&task_path, &len, stdin)) != 0) {
        task_path[nread-1]= 0;
        solve_task(task_path);
    
    }

    free(task_path);
    exit(EXIT_SUCCESS);
} 

void solve_task(char* task_path){

    char shell_minisat_line[4096];
    char minisat_required_output[4096];

    if(sprintf(shell_minisat_line, "minisat %s | grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\" | tr \"\\n\" \"\\t\" | tr -d \" \\t\" | tr -d \" \"", task_path) < 0){
        perror("sprintf error");
        exit(1);
    }

    FILE* result_task;
    if((result_task = popen(shell_minisat_line, "r")) == NULL){
        perror("popen error");
        exit(1);
    }

    if(fgets(minisat_required_output, 4096, result_task) == NULL) {
        perror("fgets");
        exit(1);
    }

    if(pclose(result_task) == -1){
        perror("error pclose");
        exit(1);
    }

    int number_of_variables, number_of_clauses;
    float cpu_time;
    char satisfiability[14] = {0};


    if(sscanf(minisat_required_output, "Numberofvariables:%dNumberofclauses:%dCPUtime:%10fs%s", &number_of_variables, &number_of_clauses, &cpu_time, satisfiability) == EOF){
        perror("sscanf");
        exit(1);
    }

    printf("PID: %d Filename: %s Number of variables: %d Number of clauses: %d CPU time: %fs %s\n", getpid(), task_path, number_of_variables, number_of_clauses, cpu_time, satisfiability);
}