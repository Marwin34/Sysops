#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>


static int state = 0;

void serveSIGTSTP(int signal_index){
    printf("Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu. \n");
    ++state;
}

void serveSIGINT(int signal){
    printf("Odebrano sygnał SIGINT. \n");
    exit(0);
}

int main(){

    signal(SIGINT, serveSIGINT);
    
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = serveSIGTSTP;
    sigaction(SIGTSTP, &sa, NULL);

    time_t t;

    while(1){

        if(state % 2 == 0){
            time(&t);
            struct tm tm = *localtime(&t);

            printf("Aktualna data: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
            usleep(1000000);
        }
    }

    return 0;
}