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
#include "my_queue.h"


int running = 1;

int X, K, M;

pid_t loaders[10];

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

void pickup_package(struct my_queue* ptr, int *loaded){
    struct package pack = top_value(ptr, X, (*loaded));

    if(pack.m != 0){
        if(pack.m > 0){
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

            nowtime2 = pack.time.tv_sec;
            nowtm2 = localtime(&nowtime2);
            strftime(tmbuf2, sizeof tmbuf2, "%Y-%m-%d %H:%M:%S", nowtm2);
            snprintf(buf2, sizeof buf2, "%s.%06ld", tmbuf2, pack.time.tv_usec);

            int u_diff = abs((tv.tv_usec - pack.time.tv_usec));
            float s_diff = tv.tv_sec - pack.time.tv_sec;

            printf("------------------------------------------------------ \n");
            printf("Took package at         %s. \n", buf);
            printf("Put at                  %s. \n", buf2);
            printf("Time diff               %.fs %dus. \n", s_diff, u_diff);
            printf("With m                  %d. \n", pack.m);
            printf("By worker with PID      %d. \n", pack.pid);
            printf("Occupied space          %d. \n", ptr->packages_count);
            printf("Free space              %d. \n", ptr->K - ptr->packages_count);
            printf("------------------------------------------------------ \n");

            if(loaders_contain(pack.pid) == -1){
                loaders_add(pack.pid);
            }

            (*loaded) += pack.m;
            printf("Package succesfully loaded. \n");
        }else if(pack.m == -2){
            (*loaded) = X;
            printf("------------------------------------------------------ \n");
            printf("Package cant be laoded and return to convoyer belt. \n");
            printf("------------------------------------------------------ \n");
        }
    }
}

int main(int argc, char** argv){

    if(signal(SIGINT, sigint_handler) == SIG_ERR){
        perror("Unnable to register SIGINT. \n");
        exit(10);
    }

    if(sscanf(argv[1], "%d", &X) != 1 || sscanf(argv[2], "%d", &M) != 1 || sscanf(argv[3], "%d", &K) != 1){
        printf("Bad arguments type. \n");
        exit(6);
    }

    struct my_queue* ptr = create_queue("/trucker");

    init_queue(ptr, M, K);

    sem_t* semaphore = create_semaphore("/trucker_sem");

    loaders_init();

    while(running == 1 || ptr->packages_count != 0){
        printf("------------------------------------------------------ \n");
        printf("Empty truck is aproaching ramp. \n");

        int loaded = 0;
        while(loaded < X){

            if(running == 0 && ptr->packages_count == 0){
                printf("All packages loaded. \n");
                break;
            }

            lock_semaphore(semaphore);
            pickup_package(ptr, &loaded);
            unlock_semaphore(semaphore);
            sleep(1);

        }

        printf("Truck is leaving ramp to be unloaded. \n");
    }

    close_queue(ptr);

    close_semaphore(semaphore);

    unlink_semaphore("/trucker_sem");

    return 0;
}