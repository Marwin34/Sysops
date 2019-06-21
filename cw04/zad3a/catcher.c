#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <wait.h>
#include <signal.h>
#include <string.h>


static int status = 0;
static int sig_cnt = 0;
static pid_t target = -1;


void handle_runtine(int s, siginfo_t *info, void *ucontext){
    //printf("Odebrano sygnal SIGUSR1. \n");
    ++sig_cnt;

    if(target == -1){
        target = info->si_pid;
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
    routine.sa_flags = SA_SIGINFO;
    routine.sa_sigaction = handle_runtine;
    sigemptyset(&routine.sa_mask);

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

    if(argc == 2){
        int type = -1;

        if (strcmp(argv[1], "kill") == 0) {
            type = 0;
        } else if (strcmp(argv[1], "sigqueue") == 0) {
            type = 1;
        } else if (strcmp(argv[1], "sigrt") == 0) {
            type = 2;
        } else {
            printf("Niepoprawny typ komunikacji. \n");
            exit(1);
        }

        printf("Jestem numer %d.\n", getpid());

        block_signals(type);
        set_up_handlers(type);

        while(status % 2 == 0){
            sleep(1);
        }

        //printf("Rozpoczynam wysylanie powrotne. \n");

        if(type == 0){
            for(int i = 0; i < sig_cnt; ++i){
                kill(target, SIGUSR1);
            }

            kill(target, SIGUSR2);
        }else if(type == 1){
            union sigval val;
            val.sival_int = 0;

            for(int i = 0; i < sig_cnt; ++i){
                val.sival_int = i;
                sigqueue(target, SIGUSR1, val);
            }

            sigqueue(target, SIGUSR2, val);
        }else if(type ==2){
             for(int i = 0; i < sig_cnt; ++i){
                kill(target, SIGRTMIN);
            }

            kill(target, SIGRTMIN+1);
        }

        printf("Catcher: otrzymano %d. \n", sig_cnt);
    }

    return 0;
}