#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <wait.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/resource.h>
#include <signal.h>
#include "backup.h"

#define MAX_SIZE 1024


static char* path_list[MAX_SIZE];
static int interval_list[MAX_SIZE];
static pid_t pid_list[MAX_SIZE];
static int monitor_list_length = 0;
static int counter = 0;
static int monitor_state = -1;
static int running = 1;

void backup(char* target){
    struct stat sb;

    if(lstat(target, &sb) == -1){
        printf("Nie mozna odczytac statystyk pliku! \n");
        exit(0);
    }

    if(memory_backup(target, sb) == 1){
        ++counter;
    }
}

void monitor(int index){
    //pid_t pid = getpid();
    char* target = path_list[index];

    int interval = interval_list[index];

    time_t spawn_time;
    time(&spawn_time);

    time_t current_time = spawn_time;

    long long last_update = (long long)(current_time - spawn_time);

    while(running == 1){
        if((long long)(current_time - spawn_time) - last_update >= interval){
            last_update = (long long)(current_time - spawn_time);
            if(monitor_state == 1){
                backup(target);
            }else {
                //printf("Stan jalowy. \n");
            }
        }
        time(&current_time);
    }
    free(target);

    exit(counter);
}

void raport(){
    for (int i = 0; i < monitor_list_length; ++i) {
        int status;
        waitpid(pid_list[i], &status, 0);
        int counter = WEXITSTATUS(status);
        printf("\nPID: %d wykonal %d kopii.", pid_list[i], counter);
    }
    printf("\n");
}

void start(pid_t target){
    union sigval val;
    sigqueue(target, SIGUSR1, val);
}

void stop(pid_t target){
    union sigval val;
    sigqueue(target, SIGUSR2, val);
}

void end(pid_t target){
    union sigval val;
    sigqueue(target, SIGINT, val);
}

void handle_SIGUSR1(int s){
    monitor_state = 1;
}

void handle_SIGUSR2(int s){
    monitor_state = 0;
}

void handle_SIGINT(int s){
    if(monitor_state == -1){
        for(int i = 0; i < monitor_list_length; ++i){
            end(i);
        }

        raport();

        exit(0);
    }else{
        running = 0;
    }
}

void parse_list(char* path){
    FILE* fp = fopen(path, "r");

    if(fp == NULL){
        printf("Nie mozna otworzyc listy.");
        exit(-1);
    }

    while (1) {
        char* target_name = calloc(4096, sizeof(char*));
        char* path = calloc(4096, sizeof(char*));
        int interval;

        int ret = fscanf(fp, "%s %s %d", target_name, path, &interval);
        if(ret == 3){
            path_list[monitor_list_length] = path;
            interval_list[monitor_list_length] = interval;
            ++monitor_list_length;
        } else if(ret == EOF) {
            free(path);
            break;
        } else {
            printf("Blad podczas parsowania danych.\n");
            exit(-1);
        }

        free(target_name);
    }
    
    fclose(fp);
}

int main(int argc, char** argv){

    if(argc == 2){
        parse_list(argv[1]);
    }

    for(int i = 0; i < monitor_list_length; ++i){
        pid_t child = fork();

        struct sigaction signal_int;
        memset(&signal_int, 0, sizeof(struct sigaction));
        signal_int.sa_handler = handle_SIGINT;
        sigaction(SIGINT, &signal_int, NULL);

        if(child == -1){
            printf("Nie mozna bylo utworzyc procesu. \n");
            return -1;
        }else if(child > 0){
            //printf("Jestem w glownym procesie! \n");

            pid_list[i] = child;
        }else{
            struct sigaction signal_usr1;
            memset(&signal_usr1, 0, sizeof(struct sigaction));
            signal_usr1.sa_handler = handle_SIGUSR1;
            sigaction(SIGUSR1, &signal_usr1, NULL);

            struct sigaction signal_usr2;
            memset(&signal_usr2, 0, sizeof(struct sigaction));
            signal_usr2.sa_handler = handle_SIGUSR2;
            sigaction(SIGUSR2, &signal_usr2, NULL);

            monitor_state = 1;

            monitor(i);
        }
    }

    char input[256];
    char delimiters[] = " \r\n\t";
    char* word;

    while(1){
        fgets(input, sizeof(input), stdin);

        word = strtok(input, delimiters);

        if(word != NULL){
            if(strcmp(word, "list") == 0){
            for(int i = 0; i < monitor_list_length; ++i){
                printf("PID %d monitoruje %s. \n", pid_list[i], path_list[i]);
            }
            }else if(strcmp(word, "end") == 0){
                for(int i = 0; i < monitor_list_length; ++i){
                    end(pid_list[i]);
                }

                raport();                

                exit(0);
            }else if(strcmp(word, "start") == 0 || strcmp(word, "stop") == 0){
                int action = strcmp(word, "start");

                word = strtok(NULL, delimiters);
                if(word == NULL){
                    printf("Nalezy podac PID or ALL. \n");
                }else {
                    int targets[MAX_SIZE];
                    int size = -1;

                    if(strcmp(word, "all") == 0){
                        for(int i = 0; i < monitor_list_length; ++i){
                            targets[i] = pid_list[i];
                        }

                        size = monitor_list_length;
                    }else{
                        char* tmp;
                        int pid = strtol(word, &tmp, 10);
                        if(tmp == word){
                            printf("Podany PID nie jest liczba! \n");
                        }else{
                            targets[0] = pid;
                            size = 1;
                        }
                    }
                    
                    if(action == 0){
                        if(size > 0){
                            for(int i = 0; i < size; ++i){
                                start(targets[i]);
                            }
                        }else{
                            printf("Blad podczas startowania! \n");
                        }
                    }else{
                        if(size > 0){
                            for(int i = 0; i < size; ++i){
                                stop(targets[i]);
                            }
                        }else{
                            printf("Blad podczas zatrzymywania! \n");
                        }
                    }
                }                    
            }else{
                printf("Unknow command! \n");
            }
        }
    }

    return 0;
}