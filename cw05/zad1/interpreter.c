#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#define MAX_COMMANDS 32
#define MAX_ARGS 16


void run(char* line){
    char* commands[MAX_COMMANDS];
    char* args[MAX_COMMANDS][MAX_ARGS];

    for(int i = 0; i < MAX_COMMANDS; ++i){
        for(int j = 0; j < MAX_ARGS; ++j){
            args[i][j] = NULL;
        }
        commands[i] = NULL;
    }

    char* program_end;
    char* program = strtok_r(line, "|", &program_end);
    
    int index = 0;
    while(program != NULL){
        char* piece_end;
        char* piece = strtok_r(program, " ", &piece_end);
        int cnt = 0;
        int args_index = 0;
        while(piece != NULL){
            if(cnt == 0){
                commands[index] = piece;
            }
            args[index][args_index] = piece;
            ++args_index;
            ++cnt;
            piece = strtok_r(NULL, " ", &piece_end);
        }
        program = strtok_r(NULL, "|", &program_end);
        ++index;
    }

    /*for(int i = 0; i < MAX_COMMANDS; ++i){
        if(commands[i] == NULL){
            break;
        }
        printf("%s", commands[i]);
        for(int j =0; j < MAX_ARGS; ++j){
            if(args[i][j] == NULL){
                break;
            }
            printf(" %s", args[i][j]);
        }
        printf("\n");
    }*/

    int pipe_count = index - 1;

    int pipes[MAX_COMMANDS][2];
    for(int i = 0; i < pipe_count; ++i){
        if(pipe(pipes[i]) == -1){
            printf("Nie mozna utworzyc potoku. \n");
            exit(1);
        }
    }

    pid_t children[index];
    for(int i = 0; i < index; ++i){
        children[i] = fork();

        if(children[i] == -1){
            printf("Blad podczas tworzenia procesu. \n");
        }else if(children[i] == 0){
            if(i > 0){
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            if(i < index - 1){
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            for(int j = 0; j < pipe_count; ++j){
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(commands[i], args[i]);

        }
    }

    for(int j = 0; j < pipe_count; ++j){
        close(pipes[j][0]);
        close(pipes[j][1]);
    }

    for(int i = 0; i < index; ++i){
        waitpid(children[i], 0, 0);
    }
}

int main(int argc, char** argv){
    if(argc == 2){
        char* path = argv[1];
        FILE* fp = fopen(path, "r");

        if(fp == NULL){
            printf("Blad podczas otwierania pliku. \n");
            exit(1);
        }

        fseek(fp, 0, SEEK_END);
        int size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        char* buffer = calloc(size + 1, sizeof(char));
        if(fread(buffer, 1, size, fp) != size){
            printf("Error podczas wczytywania danych z pliku.\n");
            exit(1);
        }

        fclose(fp);

        buffer[size + 1] = '\0';
        char* line = strtok(buffer, "\r\n"); 
        while(line != NULL){
            run(line);
            line = strtok(NULL, "\r\n");
        }

        free(buffer);
    }

    return 0;
}

/*
*/