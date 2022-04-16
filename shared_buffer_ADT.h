#ifndef SHARED_BUFFER_ADT_H
#define SHARED_BUFFER_ADT_H

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
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define SEM_PATH "/shm_sem"
#define SHM_PATH "/shm_buff"

typedef struct shared_buffer_CDT* shared_buffer_ADT;

shared_buffer_ADT create_shared_buffer(char* sem_path, char* shm_path, int shm_size);
void close_shared_buffer(shared_buffer_ADT shared_buffer);

shared_buffer_ADT open_shared_buffer(char* sem_path, char* shm_path, int shm_size);
void unlink_shared_buffer(shared_buffer_ADT shared_buffer);

void shared_buffer_send(shared_buffer_ADT shared_buffer, char* buff);
void shared_buffer_read(shared_buffer_ADT shared_buffer, char* buff);

void shared_buffer_post(shared_buffer_ADT shared_buffer);
void shared_buffer_wait(shared_buffer_ADT shared_buffer);

#endif 