#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <wait.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>

#include "utility.h"


int client_queue_id;
int id;
int server_queue_id;

int running = 1;

int input_type;

char file[4096];

void quit(){
    struct msqid_ds buf;
    if(msgctl(client_queue_id, IPC_RMID, &buf) == -1){
        perror("MSGCTL");
        exit(5);
    }
}

void process_command(char* command){
    char cmd[10];
    char args[246];
    memset(args, 0, 246);

    sscanf(command, "%s %246c", cmd, args);

    struct msgbuf msg;
    
    msg.mtype = string_to_type(cmd);

    if(msg.mtype == -1){
        if(strcmp(cmd, "FILE") == 0){
            memset(file, 0, 4096);

            strcpy(file, args);

            input_type = FILE_MODE;

            return;
        }
    }

    msg.value.id = id;
    
    if(msg.mtype == ADD || msg.mtype == DEL){
        if(strcmp(args, "") == 0){
            printf("Too few arguments. Usage example: ADD 1,2,3 \n");
            return;
        }

        char* number = strtok(args, ",");

        msg.value.argc = 0;

        while(number != NULL){
            char* end;
            int number_d = strtol(number, &end, 10);
            if(strcmp(end, number) == 0){
                perror("STRTOL ADD/DEL");
            }else{
                msg.value.argv[msg.value.argc++] = number_d;
            }

            number = strtok(NULL, ",");
        }

        msg.value.text[0] = '\0';

        if(msg.value.argc == 0){
            printf("Unnable to convert values. Usage example: ADD 1,2,3 \n");
            return;
        }
    }else if(msg.mtype == ECHO || msg.mtype == _2ALL || msg.mtype == _2FRIENDS){

        msg.value.argc = 0;
        sprintf(msg.value.text, "%s", args);

    }else if(msg.mtype == LIST || msg.mtype == STOP){

        if(strcmp(args, "") != 0){
            printf("Too many arguments. Usage example LIST or STOP");
            return;
        }

        msg.value.argc = 0;

    }else if(msg.mtype == FRIENDS){
        msg.value.argc = 0;

        if(strcmp(args, "") != 0){
            char* number = strtok(args, ",");

            while(number != NULL){
                char* end;
                int number_d = strtol(number, &end, 10);
                if(strcmp(end, number) == 0){
                    perror("STRTOL FRIENDS");
                }else{
                    msg.value.argv[msg.value.argc++] = number_d;
                }

                number = strtok(NULL, ",");
            }

            msg.value.text[0] = '\0';
        }
    }else if(msg.mtype == _2ONE){
        if(strcmp(args, "") == 0){
            printf("Too few arguments: Example usage: _2ONE 1 Hello \n");
            return;
        }

        int target_id = -1;
        char tail[230];

        memset(tail, 0, 230);

        sscanf(args, "%d %230c", &target_id, tail);

        if(target_id == -1){
            printf("Target id isn't number. Example usage: _2ONE 1 Hello \n");
            return;
        }  

        msg.value.argc = 1;
        msg.value.argv[0] = target_id;
        
        strcpy(msg.value.text, tail);
    }

    if(msgsnd(server_queue_id, &msg, sizeof(msg.value), 0) == -1){
        perror("MSGSND");
    }
}   

void serve_echo(struct msgbuf data){
    printf("SERVER RESPONSE: %s\n", data.value.text);
}

void serve_stop(struct msgbuf data){
    running = 0;
}

void serve_list(struct msgbuf data){
    printf("CLIENT LIST: %s \n", data.value.text);
}

void serve_friends(struct msgbuf data){
    printf("FRIENDS LIST: %s \n", data.value.text);
}

void serve_message(struct msgbuf data){
    printf("INCOMMING MESSAGE: %s \n", data.value.text);
}

int fetch_request(struct msgbuf data){
    switch (data.mtype)
    {
    case ECHO:
        serve_echo(data);
        return 1;
    
    case STOP:
        serve_stop(data);
        return 1;

    case LIST:
        serve_list(data);
        return 1;

    case FRIENDS:
        serve_friends(data);
        return 1;

    default:
        serve_message(data);
        return 1;
    }
    return 0;
}

void receiver(){
    struct msgbuf msg;
    while(running == 1){
        if(msgrcv(client_queue_id, &msg, sizeof(msg.value), -34, 0) == -1){
            if(errno != EINTR){
                perror("MSGRCV");
            }
        }
        else{
            fetch_request(msg);
        }
    }
}

void transmiter(){
    while(running == 1){
        char command[256]; 
        if(input_type == COMMANDLINE_MODE){
            char letter;
            int position = 0;
            while ((letter = fgetc(stdin)) != '\n') {
                command[position++] = letter;
            }
            command[position] = '\0';

            process_command(command);

        }else if(input_type == FILE_MODE){
            FILE* fp;
            char* line;
            size_t len = 0;
            ssize_t read;

            fp = fopen(file, "r");
            if (fp == NULL){
                perror("Unnable to open file. \n");
                input_type = COMMANDLINE_MODE;
                continue;
            }

            while ((read = getline(&line, &len, fp)) != -1) {
                memset(command, 0, 256);
                strcpy(command, line);
                free(line);
                process_command(command);
            }
            input_type = COMMANDLINE_MODE;
        }
    }
}

void stop_handler(int sig){
    process_command("STOP");
}

int main(){
    printf("START \n");
    
    char* homedir = getenv("HOME");

    key_t server_queue_key = ftok(homedir, 1);

    if(server_queue_key == -1){
        perror("FTOK");
        exit(1);
    }

    server_queue_id = msgget(server_queue_key, 0744);

    if(server_queue_id == -1){
        perror("UNABLE TO OPEN SERVER QUEUE!");
        exit(2);
    }

    client_queue_id = msgget(IPC_PRIVATE, 0744);

    if(client_queue_id == -1){
        perror("UNABLE TO OPEN CLIENT QUEUE!");
        exit(3);
    }

    if(atexit(quit) == -1){
        perror("ATEXIT");
        exit(4);
    }

    id = -1;

    struct msgbuf msg;
    msg.mtype = INIT;
    msg.value.id = id;

    sprintf(msg.value.text, "%d", client_queue_id);
    if(msgsnd(server_queue_id, &msg, sizeof(msg.value), 0) == -1){
        perror("MSGSND");
    }
    if(msgrcv(client_queue_id, &msg, sizeof(msg.value), INIT, 0) == -1){
        perror("MSGRCV");
    }

    id = msg.value.id;

    pid_t child = fork();

    if(child < 0){
        perror("FORK");
        exit(6);
    }else if(child > 0){
        struct sigaction sa;
        sa.sa_handler = stop_handler;
        if(sigaction(SIGINT, &sa, NULL) == -1){
            perror("SIGACTION");
            exit(7);
        }

        receiver();
        kill(child, SIGINT);
        int status;
        waitpid(child, &status, 0);
    }else{
        input_type = COMMANDLINE_MODE;
        transmiter();
    }
    return 0;
}
