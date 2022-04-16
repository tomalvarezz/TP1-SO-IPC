// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "includes.h"

//Se define la estructura de tal manera, logre el programa abstraerse del dato que se le pase.
struct shared_buffer_CDT{
    sem_t* shm_semaphore;
    char* sem_path;
    char* shm_path;
    char* shm_mapped_ptr;

    int shm_size, shm_fd;

    char* shm_write_ptr;
    char* shm_read_ptr;
};

/*
Función que crea de cero un shared_buffer con los paths de semaphore y shared memory especificados, con tamaño shm_size.
Notar que hace unlinks de sem y shm, para evitar conflictos en caso de que se haya creado dos veces con paths repetidos.

Los 3 parámetros pasados se asignan directamente a sus homónimos en la struct, y los demás datos se completan
con las funciones de sem_open, shm_open y mmap.
*/
shared_buffer_ADT create_shared_buffer(char * sem_path, char *shm_path, int shm_size){
    shared_buffer_ADT shared_buffer = malloc(sizeof(struct shared_buffer_CDT));
    shared_buffer->sem_path = sem_path;
    shared_buffer->shm_path = shm_path;
    shared_buffer->shm_size = shm_size;

    sem_unlink(shared_buffer->sem_path);
    shm_unlink(shared_buffer->shm_path);

    //Notar los flags O_CREAT y O_EXCL para crear el semáforo.
    shared_buffer->shm_semaphore = sem_open(sem_path, O_CREAT | O_EXCL, S_IWUSR | S_IRUSR, 1);
    if (shared_buffer->shm_semaphore == SEM_FAILED){
        perror("sem_open");
        exit(NOT_OK);
    } 

    //Notar los flags O_CREAT y O_EXCL para crear la shared memory.
    //Además se usa el flag O_RDWR para poder leer y escribir de la misma.
    shared_buffer->shm_fd = shm_open(shm_path, O_CREAT | O_EXCL | O_RDWR , S_IWUSR | S_IRUSR);
    if (shared_buffer->shm_fd == -1){
        perror("shm_open");
        exit(NOT_OK);
    }

    if (ftruncate(shared_buffer->shm_fd, shm_size) == -1){
        perror("ftruncate");
        exit(NOT_OK);
    }

    //Como usaremos esta función en la app para escribir al buffer, usamos el modo PROT_WRITE.
    shared_buffer->shm_mapped_ptr = mmap(NULL, shm_size, PROT_WRITE, MAP_SHARED, shared_buffer->shm_fd, 0);

    if (shared_buffer->shm_mapped_ptr == MAP_FAILED){
        perror("mmap");
        exit(NOT_OK);
    }

    shared_buffer->shm_write_ptr = shared_buffer->shm_mapped_ptr;
    shared_buffer->shm_read_ptr = shared_buffer->shm_mapped_ptr;

    return shared_buffer;
}

/*
Función que se encarga de cerrar los semáforos y memoria compartida y liberar la memoria allocada del ADT.
No se realizan unlinks pues podría querer accederse a la memoria desde otro programa.
*/
void close_shared_buffer(shared_buffer_ADT shared_buffer){
    if(munmap(shared_buffer->shm_mapped_ptr, shared_buffer->shm_size) == ERROR){
        perror("munmap");
        exit(NOT_OK);
    }
    
    if(sem_close(shared_buffer->shm_semaphore) == ERROR){
        perror("sem_close");
        exit(NOT_OK);
    } 

    if(close(shared_buffer->shm_fd) == ERROR){
        perror("close");
        exit(NOT_OK);
    }

    free(shared_buffer);  
}

/*
Función que abre un shared_buffer con los paths de semaphore y shared memory especificados, con tamaño shm_size.
Notar que NO hace unlinks de sem y shm, pues únicamente se abre una shared memory ya creada.

Los parámetros se asignan de forma análoga a create_shared_buffer.
Notar que desde como desde el ADT creado con open se leerá, queda reflejado en cambios en los flags de los open y mmap.
*/
shared_buffer_ADT open_shared_buffer(char* sem_path, char* shm_path, int shm_size){
    shared_buffer_ADT shared_buffer = malloc(sizeof(struct shared_buffer_CDT));
    shared_buffer->sem_path = sem_path;
    shared_buffer->shm_path = shm_path;
    shared_buffer->shm_size = shm_size;

    shared_buffer->shm_semaphore = sem_open(sem_path, O_RDONLY, S_IRUSR, 0);
    if (shared_buffer->shm_semaphore == SEM_FAILED){
        perror("sem_open");
        exit(NOT_OK);
    } 

    shared_buffer->shm_fd = shm_open(shm_path, O_RDONLY, S_IRUSR);
    if (shared_buffer->shm_fd == ERROR){
        perror("shm_open");
        exit(NOT_OK);
    }

    shared_buffer->shm_mapped_ptr = mmap(NULL, shm_size, PROT_READ, MAP_SHARED, shared_buffer->shm_fd, 0);
    if (shared_buffer->shm_mapped_ptr == MAP_FAILED){
        perror("mmap");
        exit(NOT_OK);
    }

    shared_buffer->shm_write_ptr = shared_buffer->shm_mapped_ptr;
    shared_buffer->shm_read_ptr = shared_buffer->shm_mapped_ptr;

    return shared_buffer;
}

/*
Función que se encarga de no solo de cerrar los semáforos y memoria compartida, sino de realizar los unlinks correspondientes
para no poder accedarla nuevamente desde otros programas, además de liberar la memoria allocada del ADT.
*/
void unlink_shared_buffer(shared_buffer_ADT shared_buffer){
    if(munmap(shared_buffer->shm_mapped_ptr, shared_buffer->shm_size) == ERROR){
        perror("munmap");
        exit(NOT_OK);
    }
    
    if(shm_unlink(shared_buffer->shm_path) == ERROR){
        perror("close");
        exit(NOT_OK);
    }
    
    if(sem_unlink(shared_buffer->sem_path) == ERROR){
        perror("close");
        exit(NOT_OK);
    }

    free(shared_buffer);  
}

/*
Funciones que se encargan de la lectura y escritura de la shared memory, utilizando los punteros designados para cada función.
*/
void shared_buffer_send(shared_buffer_ADT shared_buffer, char* buff){
    int size=strlen(buff);
    memcpy(shared_buffer->shm_write_ptr, buff,size);
    shared_buffer->shm_write_ptr[size] = '\n';
    shared_buffer->shm_write_ptr[size+1] = 0;
    shared_buffer->shm_write_ptr += (size+2);
}
void shared_buffer_read(shared_buffer_ADT shared_buffer, char* buff){
    strcpy(buff,shared_buffer->shm_read_ptr);
    int length = strlen(buff);
    shared_buffer->shm_read_ptr += (length+1);
}

/*
Funciones que se encargan de realizar los post y wait del semáforo de la shared_memory.
*/
void shared_buffer_post(shared_buffer_ADT shared_buffer){
    if(sem_post(shared_buffer->shm_semaphore) == ERROR) {
        perror("sem_post");
        exit(NOT_OK);
    }
}
void shared_buffer_wait(shared_buffer_ADT shared_buffer){
    if(sem_wait(shared_buffer->shm_semaphore) == ERROR) {
        perror("sem_wait");
        exit(NOT_OK);
    }
}