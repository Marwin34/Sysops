#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <wait.h>
#include <errno.h>
#include <sys/types.h>
#include <mqueue.h>
#include <fcntl.h>

#include "utility.h"

int id;

mqd_t client_queue_id;
mqd_t server_queue_id;

char* client_queue_name;

int running = 1;

int input_type;

char file[4096];

char message[MAX_MESSAGE_SIZE];
char response[MAX_MESSAGE_SIZE];

void quit(){

    if(mq_close(server_queue_id) == -1){
        perror("Unnable to close server queue.");
        exit(6);
    }

    if(mq_close(client_queue_id) == -1){
        perror("Unnable to close client queue.");
        exit(4);
    }

    if(mq_unlink(client_queue_name) == -1){
        perror("Unnable to unlink client queue.");
        exit(5);
    }
}

void process_command(char* command){
    char cmd[10];
    char args[MAX_MESSAGE_SIZE - 10];
    memset(args, 0, MAX_MESSAGE_SIZE - 10);
    memset(message, 0, MAX_MESSAGE_SIZE);

    sscanf(command, "%s %502c", cmd, args);

    int mtype = string_to_type(cmd);

    if(mtype == -1){
        if(strcmp(cmd, "FILE") == 0){
            memset(file, 0, 4096);

            strcpy(file, args);

            input_type = FILE_MODE;

            return;
        }
    }
    
    if(mtype == ADD || mtype == DEL){
        if(strcmp(args, "") == 0){
            printf("Too few arguments. Usage example: ADD 1,2,3 \n");
            return;
        }

        sprintf(message, "%d %d %s", mtype, id, args);
    }else if(mtype == ECHO || mtype == _2ALL || mtype == _2FRIENDS){

        sprintf(message, "%d %d %s", mtype, id, args);

    }else if(mtype == LIST || mtype == STOP){

        if(strcmp(args, "") != 0){
            printf("Too many arguments. Usage example LIST or STOP");
            return;
        }

        sprintf(message, "%d %d", mtype, id);

    }else if(mtype == FRIENDS){
        if(strcmp(args, "") == 0){
            sprintf(message, "%d %d", mtype, id);
        }else{
            sprintf(message, "%d %d %s", mtype, id, args);
        }
    }else if(mtype == _2ONE){
        if(strcmp(args, "") == 0){
            printf("Too few arguments: Example usage: _2ONE 1 Hello \n");
            return;
        }

        int target_id = -1;
        char tail[450];

        memset(tail, 0, 450);

        sscanf(args, "%d %450c", &target_id, tail);

        if(target_id == -1){
            printf("Target id isn't number. Example usage: _2ONE 1 Hello \n");
            return;
        }  
        
        sprintf(message, "%d %d %d %s", mtype, id, target_id, tail);
    }

    if(mq_send(server_queue_id, message, strlen(message), mtype) == -1){
        perror("send message");
    }
}   

void serve_echo(char* data){
    printf("SERVER RESPONSE: %s\n", data);
}

void serve_stop(char* data){
    running = 0;
}

void serve_list(char* data){
    printf("CLIENT LIST: %s \n", data);
}

void serve_friends(char* data){
    printf("FRIENDS LIST: %s \n", data);
}

void serve_message(char* data){
    printf("INCOMMING MESSAGE: %s \n", data);
}

int fetch_request(){
    int mtype;
    char data[MAX_MESSAGE_SIZE];

    memset(data, 0, 512);

    sscanf(message, "%d %512c", &mtype, data);

    switch (mtype)
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
    while(running == 1){
        memset(message, 0, MAX_MESSAGE_SIZE);
        if(mq_receive(client_queue_id, message, MAX_MESSAGE_SIZE, NULL) == -1){
            if(errno != EINTR){
                perror("MSGRCV");
            }
        }
        else{
            fetch_request();
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
    
    client_queue_name = calloc(128, sizeof(char*));

    sprintf(client_queue_name, "/client%d", getpid());

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGE_COUNT;
    attr.mq_msgsize = MAX_MESSAGE_SIZE;
    attr.mq_curmsgs = 0;

    if((client_queue_id = mq_open(client_queue_name, O_RDONLY | O_CREAT, 0744, &attr)) == -1){
        perror(client_queue_name);
        exit(1);
    }

    if((server_queue_id = mq_open("/server34", O_WRONLY)) == -1){
        perror("Unnable to open server queue.");
        exit(2);
    }

    if(atexit(quit) == -1){
        perror("ATEXIT");
        exit(3);
    }

    id = -1;

    memset(message, 0, MAX_MESSAGE_SIZE);
    memset(response, 0, MAX_MESSAGE_SIZE);

    sprintf(message, "%d %s", INIT, client_queue_name);

    if(mq_send(server_queue_id, message, strlen(message) + 1, INIT) == -1){
        perror("send INIT");
        exit(3);
    }
    if(mq_receive(client_queue_id, response, MAX_MESSAGE_SIZE, NULL) == -1){
        perror("receive INIT");
        exit(4);
    }

    int desc;
    int val;
    sscanf(response, "%d %d", &desc, &val);

    if(desc == INIT){
        id = val;
    }

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
