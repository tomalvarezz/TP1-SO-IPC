//*includes----------------------
#include "shared_buffer_ADT.h"
//*------------------------------

struct shared_buffer_CDT{
    sem_t* shm_semaphore;
    char* sem_path;
    char* shm_path;
    char* shm_mapped_ptr;

    int shm_size, shm_fd;

    char* shm_write_ptr;
    char* shm_read_ptr;
};
//se define la estructura de tal manera, logre el programa abstraerse del dato que se le pase.

shared_buffer_ADT create_shared_buffer(char * sem_path, char *shm_path, int shm_size){
    shared_buffer_ADT shared_buffer= malloc(sizeof(struct shared_buffer_CDT));
    shared_buffer->sem_path=sem_path;
    shared_buffer->shm_path=shm_path;
    shared_buffer->shm_size=shm_size;

    sem_unlink(shared_buffer->sem_path);
    shm_unlink(shared_buffer->shm_path);

    shared_buffer->shm_semaphore = sem_open(sem_path, O_CREAT | O_EXCL, 0660, 1);
    if (shared_buffer->shm_semaphore == SEM_FAILED){
        perror("sem_open");
        exit(1);
    } 

    shared_buffer->shm_fd = shm_open(shm_path, O_CREAT | O_RDWR | O_EXCL, S_IWUSR | S_IRUSR);
    if (shared_buffer->shm_fd == -1){
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(shared_buffer->shm_fd, shm_size) == -1){
        perror("ftruncate");
        exit(1);
    }

    shared_buffer->shm_mapped_ptr = mmap(NULL, shm_size, PROT_WRITE, MAP_SHARED, shared_buffer->shm_fd, 0);

    if (shared_buffer->shm_mapped_ptr == MAP_FAILED){
        perror("mmap");
        exit(1);
    }

    shared_buffer->shm_write_ptr = shared_buffer->shm_mapped_ptr;
    shared_buffer->shm_read_ptr = shared_buffer->shm_mapped_ptr;

    return shared_buffer;
}

void close_shared_buffer(shared_buffer_ADT shared_buffer){
     if(munmap(shared_buffer->shm_mapped_ptr, shared_buffer->shm_size) == -1){
        perror("munmap");
        exit(1);
    }
    
    if(sem_close(shared_buffer->shm_semaphore) == -1){
        perror("sem_close");
        exit(1);
    } 

    if(close(shared_buffer->shm_fd) == -1){
        perror("close");
        exit(1);
    }

    //!el unlink lo debemos hacer en el view porque sino no podriamos leer de la shared memory
    //!luego de haber terminado el programa

    free(shared_buffer);  
}

shared_buffer_ADT open_shared_buffer(char* sem_path, char* shm_path, int shm_size){
    shared_buffer_ADT shared_buffer = malloc(sizeof(struct shared_buffer_CDT));
    shared_buffer->sem_path=sem_path;
    shared_buffer->shm_path=shm_path;
    shared_buffer->shm_size=shm_size;

    shared_buffer->shm_semaphore = sem_open(sem_path, O_RDONLY, S_IRUSR, 0);
    if (shared_buffer->shm_semaphore == SEM_FAILED){
        perror("sem_open");
        exit(1);
    } 

    shared_buffer->shm_fd = shm_open(shm_path, O_RDONLY, S_IRUSR);
    if (shared_buffer->shm_fd == -1){
        perror("shm_open");
        exit(1);
    }

    shared_buffer->shm_mapped_ptr = mmap(NULL, shm_size, PROT_READ, MAP_SHARED, shared_buffer->shm_fd, 0);
    if (shared_buffer->shm_mapped_ptr == MAP_FAILED){
        perror("mmap");
        exit(1);
    }

    shared_buffer->shm_write_ptr = shared_buffer->shm_mapped_ptr;
    shared_buffer->shm_read_ptr = shared_buffer->shm_mapped_ptr;

    return shared_buffer;
}

void unlink_shared_buffer(shared_buffer_ADT shared_buffer){
    if(munmap(shared_buffer->shm_mapped_ptr, shared_buffer->shm_size) == -1){
        perror("munmap");
        exit(1);
    }
    
    if(shm_unlink(shared_buffer->shm_path) == -1){
        perror("close");
        exit(1);
    }
    
    if(sem_unlink(shared_buffer->sem_path) == -1){
        perror("close");
        exit(1);
    }

    free(shared_buffer);  
}

void shared_buffer_send(shared_buffer_ADT shared_buffer,char* buff){
    int size=strlen(buff);
    memcpy(shared_buffer->shm_write_ptr, buff,size);
    shared_buffer->shm_write_ptr[size]='\n';
    shared_buffer->shm_write_ptr[size+1]=0;
    shared_buffer->shm_write_ptr += (size+2);
}

int shared_buffer_read(shared_buffer_ADT shared_buffer,char* buff){
    strcpy(buff,shared_buffer->shm_read_ptr);

    int length=strlen(buff);
    shared_buffer->shm_read_ptr+=(length+1);

    return length;
}

void shared_buffer_post(shared_buffer_ADT shared_buffer){
    if(sem_post(shared_buffer->shm_semaphore) == -1) {
        perror("sem_post");
        exit(1);
    }
}

void shared_buffer_wait(shared_buffer_ADT shared_buffer){
    if(sem_wait(shared_buffer->shm_semaphore) == -1) {
        perror("sem_wait");
        exit(1);
    }
}