#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <errno.h>

#include "utility.h"


int clients[CLIENTS_SIZE];
int friend_list[CLIENTS_SIZE][CLIENTS_SIZE - 1];

static int server_queue_id;

int running = 1;

void clients_init(){
    for(int i = 0; i < CLIENTS_SIZE; ++i){
        clients[i] = -1;
    }
}

void friend_list_init(){
    for(int i = 0; i < CLIENTS_SIZE; ++i){
        for(int j = 0; j < CLIENTS_SIZE - 1; ++j){
            friend_list[i][j] = -1;
        }
    }
}

int add_client(int client_queue_id){
    for(int i = 0; i < CLIENTS_SIZE; ++i){
        if(clients[i] == -1){
            clients[i] = client_queue_id;
            return i;
        }
    }
    return -1;
}

int remove_client(int client_index){
    if(client_index > -1 && client_index < CLIENTS_SIZE){
        clients[client_index] = -1;
        return 1;
    }
    return -1;
}

int client_exist(int client_index){
    if(clients[client_index] >= 0){
        return 1;
    }else {
        return 0;
    }
}

int already_friends(int client_index, int friend_index){
    if(client_exist(client_index) == 0 || client_exist(friend_index) == 0){
        return 0;
    }

    for(int i = 0; i < CLIENTS_SIZE - 1; ++i){
        if(friend_list[client_index][i] == friend_index){
            return 1;
        }
    }

    return 0;
}

int add_friend(int client_index, int friend_index){
    if(client_index == friend_index){
        return -1;
    }

    if(client_exist(client_index) == 0 || client_exist(friend_index) == 0){
        return -1;
    }
    
    if(already_friends(client_index, friend_index) == 1){
        return -1;
    }

    for(int i = 0; i < CLIENTS_SIZE - 1; ++i){
        if(friend_list[client_index][i] == -1){
            friend_list[client_index][i] = friend_index;
            return 1;
        }
    }

    return -1;
}

int remove_friend(int client_index, int friend_index) {
    if(client_exist(client_index) == 0 && client_exist(friend_index) == 0){
        return -1;
    }

    if(already_friends(client_index, friend_index) == 0){
        return 1;
    }

    for(int i = 0; i < CLIENTS_SIZE - 1; ++i){
        if(friend_list[client_index][i] == friend_index){
            friend_list[client_index][i] = -1;
            return 1;
        }
    }

    return 1;
}

void send_friend_list(int client_index){
    struct msgbuf data;
    data.mtype = FRIENDS;
    data.value.argc = 0;
    data.value.id = client_index;

    memset(data.value.text, 0, 256);

    for(int i = 0; i < CLIENTS_SIZE - 1; ++i){
        if(friend_list[client_index][i] != -1){
            char* number = calloc(1, sizeof(char*));

            sprintf(number, "%d, ", friend_list[client_index][i]);

            strcat(data.value.text, number);
        }
    }

    if(msgsnd(clients[client_index], &data, sizeof(data.value), 0) == -1){
        perror("MSGSND to client");
        return;
    }
}

int send_response(int type, int index, char* value){
    if(client_exist(index) == 0){
        printf("Client %d doesn't exist. \n", index);
        return -1;
    }
    struct msgbuf msg;
    msg.mtype = type;
    msg.value.id = index;
    sprintf(msg.value.text, "%s", value);
    if(msgsnd(clients[index], &msg, sizeof(msg.value), 0) == -1){
        perror("MSGSND to client");
        return -1;
    }
    return 1;
}

void serve_init(struct msgbuf data){
    char* end;
    int client_queue_id = strtol(data.value.text, &end, 10);
    if(strcmp(end, data.value.text) == 0){
        perror("STRTOL");
        exit(6);
    }
    int index = add_client(client_queue_id);
    if(index == -1){
        printf("Unnable to add client_queue_id to clients %d\n.", client_queue_id);
    }
    else{
        char value[256];
        sprintf(value, "%d", index);
        send_response(INIT, index, value);
    }
}

void serve_echo(struct msgbuf data){
    send_response(ECHO, data.value.id, data.value.text);
}

void serve_stop(struct msgbuf data){
    if(client_exist(data.value.id) == 0){
        printf("Client %d doens't exist. \n", data.value.id);
    }

    send_response(STOP, data.value.id, "OK");
    remove_client(data.value.id);
} 

void serve_friends(struct msgbuf data){
    for(int i = 0; i < CLIENTS_SIZE - 1; ++i){
        friend_list[data.value.id][i] = -1;
    }

    for(int i = 0; i < data.value.argc; ++i){
        printf("A");
        friend_list[data.value.id][i] = data.value.argv[i];
    }

    send_friend_list(data.value.id);
}

void serve_add(struct msgbuf data){
    int friends_cnt = 0;
    for(int i = 0; i < CLIENTS_SIZE - 1; ++i){
        if(friend_list[data.value.id][i] != -1){
            ++friends_cnt;
        }
    }
    
    if(CLIENTS_SIZE - 1 - friends_cnt < data.value.argc){
        printf("Not enough free space in friend list.\n");
        return;
    }

    for(int i = 0; i < data.value.argc; ++i){
        if(add_friend(data.value.id, data.value.argv[i]) == -1){
            printf("Unnable to add friend %d. \n", data.value.argv[i]);
        }
    }

    send_friend_list(data.value.id);
}

void serve_del(struct msgbuf data){
    for(int i = 0; i < data.value.argc; ++i){
        if(remove_friend(data.value.id, data.value.argv[i]) == 0){
            printf("Unnable to remove friend %d. \n", data.value.argv[i]);
        }
    }

    send_friend_list(data.value.id);
}

void serve_2all(struct msgbuf data){
    for(int i = 0; i < CLIENTS_SIZE; ++i){
        if(i == data.value.id || clients[i] == -1){
            continue;
        }

        send_response(_2ALL, i, data.value.text);
    }
}

void serve_2friends(struct msgbuf data){
    for(int i = 0; i < CLIENTS_SIZE - 1; ++i){
        if(friend_list[data.value.id][i] != -1){
            send_response(_2FRIENDS, friend_list[data.value.id][i], data.value.text);
        }
    }
}

void serve_2one(struct msgbuf data){
    send_response(_2ONE, data.value.argv[0], data.value.text);
    send_response(ECHO, data.value.id, "OK");   
}

void serve_list(struct msgbuf data){
    char* message = calloc(CLIENTS_SIZE, sizeof(char*));

    int letter = 0;
    for(int i = 0; i < CLIENTS_SIZE; ++i){
        if(clients[i] != -1){
            char* piece = calloc(1, sizeof(char*));
            sprintf(piece, "%d, ", i);

            strcat(message, piece);
            ++letter;
        }
    }

    message[letter] = '\0';

    send_response(LIST, data.value.id, message);
    free(message);
}

int fetch_request(struct msgbuf data){
    switch (data.mtype)
    {
    case INIT:
        serve_init(data);
        return 1;
    
    case ECHO:
        serve_echo(data);
        return 1;

    case STOP:
        serve_stop(data);
        return 1;

    case FRIENDS:
        serve_friends(data);
        return 1;

    case ADD:
        serve_add(data);
        return 1;

    case DEL:
        serve_del(data);
        return 1;

    case _2ALL:
        serve_2all(data);
        return 1;

    case _2FRIENDS:
        serve_2friends(data);
        return 1;

    case _2ONE:
        serve_2one(data);
        return 1;

    case LIST:
        serve_list(data);
        return 1;

    default:
        break;
    }
    return 0;
}

void debug_printf_clients(){
    for(int i = 0; i < CLIENTS_SIZE; ++i){
        printf("%d -> %d \n", i, clients[i]);
    }
}

void quit(){
    struct msqid_ds buf;
    if(msgctl(server_queue_id, IPC_RMID, &buf) == -1){
        perror("SHMCTL");
        exit(5);
    }
}

void stop_handler(int sig){
    running = 0;
}

int main(){
    printf("START \n");

    struct sigaction sa;
    sa.sa_handler = stop_handler;
    if(sigaction(SIGINT, &sa, NULL) == -1){
        perror("SIGACTION");
        exit(7);
    }
    
    char* homedir = getenv("HOME");

    key_t server_queue_key = ftok(homedir, 1);

    if(server_queue_key == -1){
        perror("FTOK");
        exit(1);
    }

    server_queue_id = msgget(server_queue_key, 0744 | IPC_CREAT);

    if(server_queue_id == -1){
        perror("UNABLE TO OPEN SERVER QUEUE!");
        exit(2);
    }

    if(atexit(quit) == -1){
        perror("ATEXIT");
        exit(4);
    }

    clients_init();
    friend_list_init();

    struct msgbuf msg;
    while(running == 1){
        if(msgrcv(server_queue_id, &msg, sizeof(msg.value), -34, 0) == -1){
            if(errno != EINTR){
                perror("MSGRCV");
            }
        }
        else{
            printf("ODEBRANO %d %s\n", msg.value.id, msg.value.text);
            fetch_request(msg);
            debug_printf_clients();
        }
    }

    for(int i =0; i < CLIENTS_SIZE; ++i){
        if(clients[i] != -1){
            send_response(STOP, i, "OK");
        }
    }

    for(int i = 0; i < CLIENTS_SIZE;++i){
        remove_client(i);
    }

    return 0;
}