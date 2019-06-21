#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>


int running = 1;

pid_t* kids;

int N;

void sigint_handler(int signal){
    for(int i =0; i < N; ++i){
        kill(kids[i], SIGINT);
    }
    running = 0;
}

int main(int argc, char** argv){

    if(argc != 3){
        printf("Bad arguments count. Usage ./loader_manager N K \n");
        exit(1);
    }

    if(sscanf(argv[1], "%d", &N) != 1){
        printf("Bad argument type. \n");
        exit(2);
    }

    if(signal(SIGINT, sigint_handler) == SIG_ERR){
        perror("Unable to register SIGINT. \n");
        exit(3);
    }

    char path [4096];

    if(getcwd(path, 4096) == NULL){
        perror("Unable to get working directory. \n");
        exit(4);
    }

    strcat(path, "/loader");

    kids = calloc(N, sizeof(pid_t));
        
    for(int i = 0; i < N; ++i){
        pid_t child = fork();

        if(child > 0){
            kids[i] = child;

        }else if(child == 0){
            if(execl(path, "loader", argv[2], NULL) == -1){
                perror("Unable to execl. \n");
                exit(5);
            }
        }else{
            perror("Unable to fork. \n");
            exit(4);
        }
    }

    int status;
    for (int i = 0; i < N; ++i) {
        wait(&status);
        printf("loader %d finished with exit code %d. \n", i, WEXITSTATUS(status));
    }

    return 0;
}