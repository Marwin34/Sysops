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
#include "my_queue.h"


int running = 1;

void sigint_handler(int signal){
    running = 0;
}

void put_package(struct my_queue* ptr, int m){
    struct timeval tv;
    time_t nowtime;
    struct tm *nowtm;
    char tmbuf[64], buf[128];

    gettimeofday(&tv, NULL);
    nowtime = tv.tv_sec;
    nowtm = localtime(&nowtime);
    strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
    snprintf(buf, sizeof buf, "%s.%06ld", tmbuf, tv.tv_usec);    

    if(put_on_queue(ptr, m, getpid(), tv) == 1){
        printf("%d loaded m=%d at %s \n", getpid(), m, buf);
    }else{
        printf("Convoyer blet is full. \n");
    }
}

int main(int argc, char** argv){

    if(signal(SIGINT, sigint_handler) == SIG_ERR){
        perror("Unnable to register SIGINT. \n");
        exit(10);
    }

    struct my_queue* ptr = open_queue("/trucker");

    sem_t* semaphore = open_semaphore("/trucker_sem");

    srand(time(NULL)); 

    while(running == 1){
        int r = (rand() % 3) + 1;
        printf("Waiting for convoyer belt spot. \n");
        lock_semaphore(semaphore);
        put_package(ptr, r);
        printf("------------------------------------------------------ \n");
        unlock_semaphore(semaphore);
    }

    close_semaphore(semaphore);
    
    return 0;
}