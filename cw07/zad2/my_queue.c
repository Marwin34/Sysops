#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "my_queue.h"


struct my_queue* create_queue(char* path){
    int id = shm_open(path, O_RDWR | O_CREAT | O_EXCL, 0744);

    if(id == -1){
        perror("Unable to open shared memory. \n");
        exit(1);
    }

    if(ftruncate(id, sizeof(struct my_queue)) == -1){
        perror("Unable to truncate memory. \n");
        exit(2);
    }

    struct my_queue* ptr = (struct my_queue*)mmap(NULL, sizeof(struct my_queue), PROT_READ | PROT_WRITE, MAP_SHARED, id, 0);

    if(ptr == (void*)-1){
        perror("Unable to map shared memory. \n");
        exit(3);
    }

    ptr->path = path;
    ptr->id = id;

    return ptr;
}

struct my_queue* open_queue(char* path){
    int id = shm_open(path, O_RDWR | O_CREAT, 0744);

    if(id == -1){
        perror("Unable to open shared memory. \n");
        exit(5);
    }

    struct my_queue* ptr = (struct my_queue*)mmap(NULL, sizeof(struct my_queue), PROT_READ | PROT_WRITE, MAP_SHARED, id, 0);

    if(ptr == (void*)-1){
        perror("Unable to map shared memory. \n");
        exit(4);
    }

    return ptr;
}

void close_queue(struct my_queue* ptr){
    char* path = ptr->path;

    if(munmap(ptr, sizeof(struct my_queue)) == -1){
        perror("Unnable to munmap. \n");
        exit(5);
    }

    if(shm_unlink(path) == -1){
        perror("Unnable to shm_unlink. \n");
        exit(6);
    }
}

void init_queue(struct my_queue* ptr, int M, int K){
    ptr->M = M;
    ptr->K = K;
    ptr->packages_count = 0;
    ptr->packages_mass = 0;

    struct package bytes;
    bytes.m = 0;
    bytes.pid = -1;
    
    for(int i = 0; i < ptr->K; ++i){
        ptr->packages[i] = bytes;
    }
}   

sem_t* create_semaphore(char* sem_name){
    sem_t* result;
    if((result = sem_open(sem_name, O_RDWR | O_CREAT | O_EXCL, 0744, 1)) == SEM_FAILED){
        perror("Unnable to sem_open. \n");
        exit(11);
    }
    return result;
}

sem_t* open_semaphore(char* sem_name){
    sem_t* result;
    if((result = sem_open(sem_name, O_RDWR, 0744)) == SEM_FAILED){
        perror("Unnable to sem_open. \n");
        exit(11);
    }
    return result;
}

void lock_semaphore(sem_t* id){
    if (sem_wait(id) == -1) {
        perror("Unable to lock semaphore. \n");
        exit(12);
    }
}

void unlock_semaphore(sem_t* id) {
    if (sem_post(id) == -1) {
        perror("Unable to unlock semaphore. \n");
        exit(13);
    }
}

void close_semaphore(sem_t* id){
    if(sem_close(id) == -1){
        perror("Unable to close semaphore. \n");
        exit(14);
    }
}

void unlink_semaphore(char* name){
    if(sem_unlink(name) == -1){
        perror("Unable to unlink semaphore. \n");
        exit(15);
    }
}

void shift(struct my_queue* ptr){
    for(int i = 0; i < ptr->K - 1; ++i){
        ptr->packages[i] = ptr->packages[i + 1];
    }
    ptr->packages[ptr->K - 1].m = 0;
    ptr->packages[ptr->K - 1].pid = -1;
}

struct package top_value(struct my_queue* ptr, int max_payload, int current_payload){

    struct package results;

    if(ptr->packages[0].m == 0){
        results.m = 0;
        results.pid = -1;
    }else if(ptr->packages[0].m + current_payload > max_payload){
        results.m = -2;
        results.pid = -1;
    }else {
        results = ptr->packages[0];

        ptr->packages_count -= 1;
        ptr->packages_mass -= ptr->packages[0].m;

        shift(ptr);
    }

    return results;
}

int put_on_queue(struct my_queue* ptr, int val, pid_t PID, struct timeval ttime){
    if(val + ptr->packages_mass > ptr->M || ptr->packages_count == ptr->K){
        return -1;
    }    
    for(int i = 0; i < ptr->K; ++i){
        if(ptr->packages[i].m == 0){
            ptr->packages[i].m = val;
            ptr->packages[i].pid = PID;
            ptr->packages[i].time = ttime;

            //printf("%d %d \n", i, ptr->packages[i].m);

            ptr->packages_mass += val;
            ptr->packages_count += 1;
            break;
        }
    }

    return 1;
}
