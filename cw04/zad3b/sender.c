#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <wait.h>
#include <signal.h>
#include <string.h>


static int status = 0;
//static int received_cnt = 0;
static int type = -1;
static int sig_cnt = 0;
static int sended = 1;
static int target = -1;

void handle_runtine(int s){
    //printf("Odebrano sygnal SIGUSR1. \n");
    if(sended < sig_cnt){
        ++sended;
        if(type == 0){
            kill(target, SIGUSR1);
        }else if(type == 1){
            union sigval val;
            val.sival_int = sended;
            sigqueue(target, SIGUSR1, val);
        }else if(type == 2){
            kill(target, SIGRTMIN);
        }
    }else{
        if(type == 0){
            kill(target, SIGUSR2);
        }else if(type == 1){
            union sigval val;
            val.sival_int = 0;
            sigqueue(target, SIGUSR2, val);
        }else if(type == 2){
            kill(target, SIGRTMIN + 1);
        }
    }
}

void handle_end(int s){
    //printf("Odebrano sygnal SIGUSR2. \n");
    ++status;
}

void block_signals(int type){
    sigset_t mask;
    sigfillset(&mask);
    if (type == 0 || type == 1) {
        sigdelset(&mask, SIGUSR1);
        sigdelset(&mask, SIGUSR2);
    } else {
        sigdelset(&mask, SIGRTMIN);
        sigdelset(&mask, SIGRTMIN+1);
    }

    if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
        printf("Nie mozna zablokowac sygnalow! \n");
        exit(1);
    }
}

void set_up_handlers(int type){
    struct sigaction routine;
    memset(&routine, 0, sizeof(struct sigaction));
    routine.sa_handler = handle_runtine;

    struct sigaction end;
    memset(&end, 0, sizeof(struct sigaction));
    end.sa_handler = handle_end;

    if (type == 0 || type == 1) {
        sigaddset(&routine.sa_mask, SIGUSR1);
        sigaddset(&routine.sa_mask, SIGUSR2);
        sigaddset(&end.sa_mask, SIGUSR1);
        sigaddset(&end.sa_mask, SIGUSR2);
        sigaction(SIGUSR1, &routine, NULL);
        sigaction(SIGUSR2, &end, NULL);
    } else {
        sigaddset(&routine.sa_mask, SIGRTMIN);
        sigaddset(&routine.sa_mask, SIGRTMIN+1);
        sigaddset(&end.sa_mask, SIGRTMIN);
        sigaddset(&end.sa_mask, SIGRTMIN+1);
        sigaction(SIGRTMIN, &routine, NULL);
        sigaction(SIGRTMIN+1, &end, NULL);
    }
}

int main(int argc, char** argv){
    if(argc == 4){
        
        char* end;
        target = strtol(argv[1], &end, 10);
        sig_cnt = strtol(argv[2], &end, 10);
        type = -1;
        
        if(strcmp(argv[3], "kill") == 0){
            type = 0;
            
        }else if(strcmp(argv[3], "sigqueue") == 0){
            type = 1;
            
        }else if(strcmp(argv[3], "sigrt") == 0){
            type = 2;
        }

        block_signals(type);
        set_up_handlers(type);

        if(type == 0){
            kill(target, SIGUSR1);
        }else if(type == 1){;
            union sigval val;
            val.sival_int = sended;

            sigqueue(target, SIGUSR1, val);
        }else if(type == 2){
            kill(target, SIGRTMIN);

        }else {
            printf("Niepoprawny typ komunikacji. \n");
            exit(1);
        }
        
        while(status % 2 == 0){
            sleep(1);        
        }

        printf("Sender: wyslano %d, otrzymano %d. \n", sended, sended);
    }
    return 0;
}