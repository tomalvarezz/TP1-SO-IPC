#ifndef SHARED_DATA_H
#define SHARED_DATA_H

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

typedef struct shared_buffer_CDT* shared_buffer_ADT;

shared_buffer_ADT create_shared_buffer(char* sem_path, char* shm_path, int shm_size);
void unlink_buff(shared_buffer_ADT);
void shm_send(shared_buffer_ADT shared_buffer, char* buff);
void close_shared_buff(shared_buffer_ADT shared_buffer);

#endif 