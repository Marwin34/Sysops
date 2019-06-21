#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <wait.h>
#include <signal.h>
#include <string.h>


static int state = 0;
static pid_t child;

void handle_SIGINT(int s){
    printf("\nOdebrano sygnał SIGINT. \n");

    if(child > 0){
        printf("Zabijam dziecko %d. \n", child);
        kill(child, SIGKILL);
    }

    printf("Koncze prace. \n");
    
    exit(0);
}

void handle_SIGTSTP(int s){
    printf("\nOczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu. \n");
    if(child > 0 && state % 2 == 0){
        printf("Zabijam dziecko %d. \n", child);
        kill(child, SIGKILL);
    }
    ++state;
}

int main(){
    printf("Jestem %d. \n", getpid());
    signal(SIGINT, handle_SIGINT);

    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = handle_SIGTSTP;
    sigaction(SIGTSTP, &sa, NULL);

    while(1){
        if(state % 2 == 0){
            child = fork();

            if(child == -1){
                printf("Nie mozna utworzyc procesu. \n");
                exit(-1);
            }else if(child > 0){
                printf("Czekam na dziecko %d. \n", child);
                int status;
                waitpid(child, &status, 0);
            }else {
                execlp("bash", "bash", "date.sh", NULL);
            }
        }
    }

    return 0;
}