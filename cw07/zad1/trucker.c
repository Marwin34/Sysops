#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include "shared.h"


int running = 1;

int X, M, K;
key_t memory_key;
key_t semaphores_key;
int memory_id;
int semaphores_id;
struct package* ptr = NULL;
int occupied = 0;

pid_t loaders[10];

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO */
};

void loaders_init(){
    for(int i = 0; i < 10; ++i){
        loaders[i] = -1;
    }
}

int loaders_contain(pid_t pid){
    for(int i = 0; i < 10; ++i){
        if(loaders[i] == pid){
            return 1;
        }
    }

    return -1;
}

void loaders_add(pid_t pid){
    if(pid == 0){
        return;
    }
    for(int i = 0; i < 10; ++i){
        if(loaders[i] == -1){
            loaders[i] = pid;
            break;
        }
    }
}

void sigint_loaders(){
    for(int i = 0; i < 10; ++i){
        if(loaders[i] > 0){
            kill(loaders[i], SIGINT);
        }
    }
}

void sigint_handler(int signal){
    sigint_loaders(); 

    running = 0;
}

void lock_tape(){
    struct sembuf actions[1];
    actions[0].sem_num = 2;
    actions[0].sem_op = -1;
    actions[0].sem_flg = SEM_UNDO;

    if(semop(semaphores_id, actions, 1) == -1){
        perror("Unable to lock convoyer belt semaphore. \n");
        exit(10);
    }
}

void unlock_tape(int m){
    if(m != 0){
        struct sembuf actions[3];
        actions[0].sem_num = 2;
        actions[0].sem_op = 1;
        actions[0].sem_flg = SEM_UNDO;

        actions[1].sem_num = 0;
        actions[1].sem_op = m;
        actions[1].sem_flg = SEM_UNDO;

        actions[2].sem_num = 1;
        actions[2].sem_op = 1;
        actions[2].sem_flg = SEM_UNDO;

        if(semop(semaphores_id, actions, 3) == -1){
            perror("Unable to lock convoyer belt semaphore. \n");
            exit(10);
        }
    }else {
        struct sembuf actions[1];
        actions[0].sem_num = 2;
        actions[0].sem_op = 1;
        actions[0].sem_flg = SEM_UNDO;

        if(semop(semaphores_id, actions, 1) == -1){
            perror("Unable to lock convoyer belt semaphore. \n");
            exit(10);
        }
    }
}

int pickup_package(int *loaded){
    occupied = 0;
    for(int i = 0; i < K; ++i){
        if(ptr[i].m != 0){
            ++occupied;
        }
    }

    int val = ptr[0].m;

    if(val != 0){
        struct timeval tv;
        time_t nowtime;
        struct tm *nowtm;
        char tmbuf[64], buf[128];

        gettimeofday(&tv, NULL);
        nowtime = tv.tv_sec;
        nowtm = localtime(&nowtime);
        strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
        snprintf(buf, sizeof buf, "%s.%06ld", tmbuf, tv.tv_usec);

        time_t nowtime2;
        struct tm *nowtm2;
        char tmbuf2[64], buf2[128];

        nowtime2 = ptr[0].time.tv_sec;
        nowtm2 = localtime(&nowtime2);
        strftime(tmbuf2, sizeof tmbuf2, "%Y-%m-%d %H:%M:%S", nowtm2);
        snprintf(buf2, sizeof buf2, "%s.%06ld", tmbuf2, ptr[0].time.tv_usec);

        int u_diff = abs((tv.tv_usec - ptr[0].time.tv_usec));
        float s_diff = tv.tv_sec - ptr[0].time.tv_sec;

        printf("------------------------------------------------------ \n");
        printf("Took package at         %s. \n", buf);
        printf("Put at                  %s. \n", buf2);
        printf("Time diff               %.fs %dus. \n", s_diff, u_diff);
        printf("With m                  %d. \n", ptr[0].m);
        printf("By worker with PID      %d. \n", ptr[0].pid);
        printf("Occupied space          %d. \n", occupied);
        printf("Free space              %d. \n", K - occupied);
        printf("------------------------------------------------------ \n");

        if(loaders_contain(ptr[0].pid) == -1){
            loaders_add(ptr[0].pid);
        }

        if((*loaded) + val <= X){
            (*loaded) += val;

            for(int i = 0; i < K - 1; ++i){
                ptr[i] = ptr[i + 1];
            }
            ptr[K - 1].m = 0;
            ptr[K - 1].pid = -1;
            printf("Package succesfully loaded. \n");
        }else if((*loaded) + val > X){
            (*loaded) = X;
            printf("Package cant be laoded and return to convoyer belt. \n");
        }
    }

    return val;
}

int main(int argc, char** argv){

    if(signal(SIGINT, sigint_handler) == SIG_ERR){
        perror("Unnable to register SIGINT. \n");
        exit(10);
    }

    if(argc != 4){  
        printf("Bad arguments count. Usage ./trucker X M K \n");
        exit(1);
    }

    if((memory_key = ftok(getenv("HOME"), 3113)) == -1){
        perror("Unable to ftok memory key \n");
        exit(2);
    }

    if((semaphores_key = ftok(getenv("HOME"), 13971)) == -1){
        perror("Unable to ftok semaphores key \n");
        exit(5);
    }

    if(sscanf(argv[1], "%d", &X) != 1 || sscanf(argv[2], "%d", &M) != 1 || sscanf(argv[3], "%d", &K) != 1){
        printf("Bad arguments type. \n");
        exit(6);
    }

    if((memory_id = shmget(memory_key, K * sizeof(struct package), IPC_CREAT | IPC_EXCL | 0744)) == -1){
        perror("Unable to create shared memory. \n");
        exit(3);
    }

    if((ptr = shmat(memory_id, ptr, SHM_R | SHM_W)) == NULL){
        perror("Unable to open shared memory. \n");
        exit(4);
    }

    if((semaphores_id = semget(semaphores_key, 3, IPC_CREAT | IPC_EXCL | 0744)) == -1){
        perror("Unable to create semaphores \n");
        exit(7);
    }

    union semun arg;
    arg.val = M;

    if(semctl(semaphores_id, 0, SETVAL, arg) == -1){
        perror("Unnable to initialize M semafor. \n");
        exit(8);
    }

    arg.val = K;

    if(semctl(semaphores_id, 1, SETVAL, arg)== -1){
        perror("Unnable to initialize K semafor. \n");
        exit(9);
    }

    arg.val = 1;

    if(semctl(semaphores_id, 2, SETVAL, arg)== -1){
        perror("Unnable to initialize semafor. \n");
        exit(9);
    }

    struct package bytes;
    bytes.m = 0;
    bytes.pid = -1;
    
    for(int i = 0; i < K; ++i){
        ptr[i] = bytes;
    }

    loaders_init();

    while(running == 1 || occupied != 0){
        printf("------------------------------------------------------ \n");
        printf("Empty truck is aproaching ramp. \n");

        int loaded = 0;
        while(loaded < X){

            if(running == 0 && occupied == 0){
                printf("All packages loaded. \n");
                break;
            }

            lock_tape();
            int val = pickup_package(&loaded);
            unlock_tape(val);
            sleep(1);

        }
        printf("------------------------------------------------------ \n");
        printf("Truck is leaving ramp to be unloaded. \n");
    }

    if(semctl(semaphores_id, 0, IPC_RMID) == -1){
        perror("Unnable to remove semaphores. \n");
        exit(11);
    }

    if(shmctl(memory_id, IPC_RMID, NULL) == -1){
        perror("Unnable to remove shared memory. \n");
        exit(12);
    }

    return 0;
}