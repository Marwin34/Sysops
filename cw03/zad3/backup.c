#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <wait.h>
#include <sys/stat.h>
#include <string.h>

#define PATH_MAX 4096


static char* file_buffer;
static int buffer_size;
static time_t last_modyfication = -1;

void load_to_buffer(char* target, struct stat sb){
    buffer_size = sb.st_size;
    file_buffer = calloc(1, buffer_size);
    

    FILE* fp = fopen(target, "r");

    if(fp == NULL){
        printf("Nie mozna otworzyc pliku %s. \n", target);
        exit(0);
    }

    fread(file_buffer, buffer_size, 1, fp);
    fclose(fp);
}

char* generate_backup_name(char* target){
    char* result = calloc(PATH_MAX, sizeof(char));
    if(last_modyfication == -1){
        printf("Data modyfikacji nie zostala wczytana! \n");
        return NULL;
    }else{
        struct tm* timeinfo;
        timeinfo = localtime(&last_modyfication);

        char date[80];
        strftime(date, sizeof(date), "%Y-%m-%d_%H-%M-%S", timeinfo);
                    
        sprintf(result, "%s_%s", target, date);
        return result;
    }
}

int memory_backup(char* target, struct stat sb){
    if(last_modyfication == -1){
        load_to_buffer(target, sb);
        last_modyfication = sb.st_atime;
    }

    if(last_modyfication != sb.st_mtime){
        printf("Jakas modyfikacja! \n");

        if(chdir("archiwum") == -1){
            printf("Nie mozna otworzyc katalogu archiwum! /n");
            exit(0);
        }

        char* copy_path = generate_backup_name(target);

        FILE * copy = fopen(copy_path, "a");

        if(copy == NULL){
            printf("Nie mozna otworzyc pliku %s. \n", copy_path);
            exit(0);
        }

        fwrite(file_buffer, buffer_size, 1, copy);
        fclose(copy);
        chdir("..");

        load_to_buffer(target, sb);

        last_modyfication = sb.st_mtime;
        return 1;
    }
    return 0;
}

int execute_backup(char* target, struct stat sb){
     if(last_modyfication != sb.st_mtime){
        last_modyfication = sb.st_mtime;
        char* backup_name = generate_backup_name(target);

        pid_t child = fork();

        if(child == -1){
            printf("Nie mozna bylo utworzyc procesu. \n");
            exit(0);
        }else if(child > 0){
            int status;
            waitpid(child, &status, 0);
        }else {
            execlp("cp", "cp", target, backup_name, NULL);
        }

        free(backup_name);
        return 1;
    }
    return 0;
}