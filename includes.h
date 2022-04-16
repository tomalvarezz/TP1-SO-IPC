#ifndef INCLUDES_H
#define INCLUDES_H

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <limits.h>

#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/select.h>

#include "shared_buffer_ADT.h"

#define MAX_WORKERS_CAN_CREATE 5
#define MAX_TASKS_PER_WORKER 3
#define MAX_BUFF 4096
#define ERROR -1
#define OK 0
#define NOT_OK 1

#endif