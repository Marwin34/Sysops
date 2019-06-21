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
#include "backup.h"

#define MAX_SIZE 1024


static char* path_list[MAX_SIZE];
static int interval_list[MAX_SIZE];
static int monitor_list_length = 0;
static int life_span;
static int counter = 0;
static int backup_type;

static int cpu_time_limit;
static int bytes_limit;

void raport_usage(){
    struct rusage usage;
    if(getrusage(RUSAGE_CHILDREN, &usage) == -1){
        printf("Blad podczas pobierania zuzycia zasobow! \n");
        exit(-1);
    }

    double user_time = usage.ru_utime.tv_sec * 1.0 +    usage.ru_utime.tv_usec / 1000000.0;
    double system_time = usage.ru_stime.tv_sec * 1.0 + usage.ru_stime.tv_usec / 1000000.0;

    printf("Czas systemowy: %.6fs, czas uzytkownika: %6fs, pamiec wspoldzielona: %ld, odebrane sygnaly: %ld. \n", 
            system_time, user_time, usage.ru_ixrss, usage.ru_nsignals);
}

void backup(char* target){
    struct stat sb;

    if(lstat(target, &sb) == -1){
        printf("Nie mozna odczytac statystyk pliku! \n");
        exit(0);
    }

    if(backup_type == 0){
        if(memory_backup(target, sb) == 1){
            ++counter;
        }

    }else if(backup_type == 1){
        if(execute_backup(target, sb) == 1){
            ++counter;
        }
    }else{
        printf("Nieznany typ backupu %d.\n", backup_type);
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

    while((long long)(current_time - spawn_time) <= life_span){
        if((long long)(current_time - spawn_time) - last_update >= interval){
            last_update = (long long)(current_time - spawn_time);
            backup(target);
        }
        time(&current_time);
    }
    free(target);

    raport_usage();

    exit(counter);
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

    if(argc == 6){
        parse_list(argv[1]);
        life_span = atoi(argv[2]);
        backup_type = atoi(argv[3]);
        cpu_time_limit = atoi(argv[4]);
        bytes_limit = atoi(argv[5]);
    }

    pid_t children[monitor_list_length];
    for(int i = 0; i < monitor_list_length; ++i){
        pid_t child = fork();

        if(child == -1){
            printf("Nie mozna bylo utworzyc procesu. \n");
            return -1;
        }else if(child > 0){
            //printf("Jestem w glownym procesie! \n");
            children[i] = child;
        }else{
            struct rlimit memory;

            memory.rlim_cur = bytes_limit;
            memory.rlim_max = bytes_limit;

            struct rlimit cpu;

            cpu.rlim_cur = cpu_time_limit;
            cpu.rlim_max = cpu_time_limit;

            if(setrlimit(RLIMIT_CPU, &cpu) == -1){
                printf("Nie mozna bylo nalozyc restrykcji na cpu!. \n");
                return -1;
            }

            if(setrlimit(RLIMIT_DATA, &memory) == -1){
                printf("Nie mozna bylo nalozyc ograniczen na pamiec wirtualna! \n");
                return -1;
            }
            monitor(i);
        }
    }

    for (int i = 0; i < monitor_list_length; ++i) {
        int status;
        waitpid(children[i], &status, 0);
        int counter = WEXITSTATUS(status);
        printf("PID: %d wykonal %d kopii.\n", children[i], counter);
    }

    return 0;
}