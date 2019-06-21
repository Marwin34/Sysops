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
#include "shared.h"


int running = 1;

key_t memory_key;
key_t semaphores_key;
int memory_id;
int semaphores_id;
int K;
struct package* ptr = NULL;

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO */
};

void sigint_handler(int signal){
    running = 0;
}

void lock_tape(int m){
    struct sembuf actions[3];
    actions[0].sem_num = 2;
    actions[0].sem_op = -1;
    actions[0].sem_flg = SEM_UNDO;

    actions[1].sem_num = 1;
    actions[1].sem_op = -1;
    actions[1].sem_flg = SEM_UNDO;

    actions[2].sem_num = 0;
    actions[2].sem_op = -m;
    actions[2].sem_flg = SEM_UNDO;

    if(semop(semaphores_id, actions, 3) == -1){
        perror("Unable to lock tape semaphore. \n");
        exit(10);
    }
}

void unlock_tape(){
    struct sembuf actions[1];
    actions[0].sem_num = 2;
    actions[0].sem_op = 1;
    actions[0].sem_flg = SEM_UNDO;

    if(semop(semaphores_id, actions, 1) == -1){
        perror("Unable to lock tape semaphore. \n");
        exit(10);
    }
}

void put_package(int m){
    
    struct package pack;
    pack.m = m;
    pack.pid = getpid();

    time_t nowtime;
    struct tm *nowtm;
    char tmbuf[64], buf[128];

    gettimeofday(&pack.time, NULL);
    nowtime = pack.time.tv_sec;
    nowtm = localtime(&nowtime);
    strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
    snprintf(buf, sizeof buf, "%s.%06ld", tmbuf, pack.time.tv_usec);    

    printf("%d loaded m=%d at %s \n", getpid(), m, buf);

    for(int i = 0; i < K; ++i){
        if(ptr[i].m == 0){    
            ptr[i++] = pack;
            break;
        }
    }
}

int main(int argc, char** argv){

    if(argc != 2){
        printf("Bad arguments count %d. Usage ./loader K \n", argc);
        exit(1);
    }

    if(sscanf(argv[1], "%d", &K) != 1){
        printf("Bad argument type. \n");
        exit(6);
    }

    if(signal(SIGINT, sigint_handler) == SIG_ERR){
        perror("Unnable to register SIGINT. \n");
        exit(10);
    }

    if((memory_key = ftok(getenv("HOME"), 3113)) == -1){
        perror("Unable to ftok memory key \n");
        exit(2);
    }

    if((semaphores_key = ftok(getenv("HOME"), 13971)) == -1){
        perror("Unable to ftok semaphores key \n");
        exit(5);
    }

    if((memory_id = shmget(memory_key, 5, 0)) == -1){
        perror("Unable to create shared memory. \n");
        exit(3);
    }

    if((ptr = shmat(memory_id, ptr, SHM_R | SHM_W)) == NULL){
        perror("Unable to open shared memory. \n");
        exit(4);
    }

    if((semaphores_id = semget(semaphores_key, 2, 0)) == -1){
        perror("Unable to create semaphores \n");
        exit(7);
    }

    srand(time(NULL)); 

    while(running == 1){
        int r = (rand() % 3) + 1;
        printf("Waiting for convoyer belt spot. \n");
        lock_tape(r);
        printf("Loading package. \n");
        printf("------------------------------------------------------ \n");
        put_package(r);
        unlock_tape();
        usleep(1000);
    }

    return 0;
}