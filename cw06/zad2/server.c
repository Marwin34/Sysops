#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <mqueue.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include "utility.h"


mqd_t clients[CLIENTS_SIZE];
int friend_list[CLIENTS_SIZE][CLIENTS_SIZE - 1];

mqd_t server_queue_id;

char message[MAX_MESSAGE_SIZE];

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
    char list[MAX_MESSAGE_SIZE - 20];

    memset(list, 0, MAX_MESSAGE_SIZE - 20);

    for(int i = 0; i < CLIENTS_SIZE - 1; ++i){
        if(friend_list[client_index][i] != -1){
            char* number = calloc(1, sizeof(char*));

            sprintf(number, "%d, ", friend_list[client_index][i]);

            strcat(list, number);
        }
    }

    char response[MAX_MESSAGE_SIZE];

    memset(response, 0, MAX_MESSAGE_SIZE);

    sprintf(response, "%d %s", FRIENDS, list);

    if(mq_send(clients[client_index], response, strlen(response), FRIENDS) == -1){
        perror("mq_send to client");
        return;
    }
}

int send_response(int type, int index, char* value){
    if(client_exist(index) == 0){
        printf("Client %d doesn't exist. \n", index);
        return -1;
    }
    char response[MAX_MESSAGE_SIZE];

    memset(response, 0, MAX_MESSAGE_SIZE);

    sprintf(response, "%d %s", type, value);

    printf("SENDING %s \n", response);

    if(mq_send(clients[index], response, strlen(response), type) == -1){
        perror("mq_send to client");
        return -1;
    }
    return 1;
}

void serve_init(char* data){
    int client_queue_id = -1;

    if((client_queue_id = mq_open(data, O_WRONLY)) == -1){
        perror("Unable to open cient queue.");
        exit(7);
    }

    int index = add_client(client_queue_id);

    if(index == -1){
        printf("Unable to add client_queue_id to clients %d\n.", client_queue_id);
    }else{
        char value[256];
        memset(value, 0, 256);
        sprintf(value, "%d", index);
        send_response(INIT, index, value);
    }
}

void serve_echo(char* data){
    int id;
    char rest[512];

    memset(rest, 0, 512);

    sscanf(data, "%d %512c", &id, rest);

    send_response(ECHO, id, rest);
}

void serve_stop(char* data){
    int client_id;
    sscanf(data, "%d", &client_id);
    if(client_exist(client_id) == 0){
        printf("Client %d doens't exist. \n", client_id);
    }

    send_response(STOP, client_id, "OK");

    if(mq_close(clients[client_id]) == -1){
        perror("Unable to close client queue.");
    }

    remove_client(client_id);
} 

void serve_friends(char* data){
    int client_id;
    char rest[MAX_MESSAGE_SIZE];

    memset(rest, 0, MAX_MESSAGE_SIZE);

    sscanf(data, "%d %512c", &client_id, rest);

    for(int i = 0; i < CLIENTS_SIZE - 1; ++i){
        friend_list[client_id][i] = -1;
    }

    char* number = strtok(rest, ",");

    int i = 0;

    while(number != NULL){
        char* end;
        int number_d = strtol(number, &end, 10);
        if(strcmp(end, number) == 0){
            perror("STRTOL friends");
        }else{
            friend_list[client_id][i++] = number_d;
        }

        number = strtok(NULL, ",");
    }

    send_friend_list(client_id);
}

void serve_add(char* data){
    int client_id;
    char rest[MAX_MESSAGE_SIZE];

    memset(rest, 0, MAX_MESSAGE_SIZE);

    sscanf(data, "%d %512c", &client_id, rest);

    for(int i = 0; i < CLIENTS_SIZE - 1; ++i){
        friend_list[client_id][i] = -1;
    }

    char* number = strtok(rest, ",");

        while(number != NULL){
            char* end;
            int number_d = strtol(number, &end, 10);
            if(strcmp(end, number) == 0){
                perror("STRTOL ADD/DEL");
            }else{
                if(add_friend(client_id, number_d) == -1){
                    printf("Unable to add friend %d. \n", client_id);
                }
            }

            number = strtok(NULL, ",");
        }

    send_friend_list(client_id);
}

void serve_del(char* data){
    int client_id;
    char rest[MAX_MESSAGE_SIZE];

    memset(rest, 0, MAX_MESSAGE_SIZE);

    sscanf(data, "%d %512c", &client_id, rest);

    for(int i = 0; i < CLIENTS_SIZE - 1; ++i){
        friend_list[client_id][i] = -1;
    }

    char* number = strtok(rest, ",");

        while(number != NULL){
            char* end;
            int number_d = strtol(number, &end, 10);
            if(strcmp(end, number) == 0){
                perror("STRTOL ADD/DEL");
            }else{
                if(remove_friend(client_id, number_d) == -1){
                    printf("Unable to add friend %d. \n", client_id);
                }
            }

            number = strtok(NULL, ",");
        }

    send_friend_list(client_id);
}

void serve_2all(char* data){
    int client_id;
    char rest[MAX_MESSAGE_SIZE];

    memset(rest, 0, MAX_MESSAGE_SIZE);

    sscanf(data, "%d %512c", &client_id, rest);

    for(int i = 0; i < CLIENTS_SIZE; ++i){
        if(i == client_id || clients[i] == -1){
            continue;
        }

        send_response(_2ALL, i, rest);
    }
}

void serve_2friends(char*  data){
    int client_id;
    char rest[MAX_MESSAGE_SIZE];

    memset(rest, 0, MAX_MESSAGE_SIZE);

    sscanf(data, "%d %512c", &client_id, rest);

    for(int i = 0; i < CLIENTS_SIZE - 1; ++i){
        if(friend_list[client_id][i] != -1){
            send_response(_2FRIENDS, friend_list[client_id][i], rest);
        }
    }
}

void serve_2one(char* data){
    int client_id;
    int target_id;
    char rest[MAX_MESSAGE_SIZE];

    memset(rest, 0, MAX_MESSAGE_SIZE);

    sscanf(data, "%d %d %512c", &client_id, &target_id, rest);

    send_response(_2ONE, target_id, rest);
    send_response(ECHO, client_id, "OK");   
}

void serve_list(char* data){
    int client_id;
    sscanf(data, "%d", &client_id);

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

    send_response(LIST, client_id, message);
    free(message);
}

int fetch_request(){

    int mtype;
    char data[512];

    memset(data, 0, 512);

    sscanf(message, "%d %512c", &mtype, data);

    switch (mtype)
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
    if(mq_close(server_queue_id) == -1){
        perror("Unable to close server queue.");
        exit(8);
    }

    if(mq_unlink("/server34") == -1){
        perror("Unable to unlink server queue.");
        exit(9);
    }
}

void stop_handler(int sig){
    running = 0;
}

int main(){
    struct sigaction sa;
    sa.sa_handler = stop_handler;
    if(sigaction(SIGINT, &sa, NULL) == -1){
        perror("SIGACTION");
        exit(7);
    }

    printf("START \n");

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGE_COUNT;
    attr.mq_msgsize = MAX_MESSAGE_SIZE;
    attr.mq_curmsgs = 0;

    if((server_queue_id = mq_open("/server34", O_RDONLY | O_CREAT, 0744, &attr)) == -1){
        perror("UNABLE TO OPEN SERVER QUEUE!");
        exit(1);
    }

    if(atexit(quit) == -1){
        perror("ATEXIT");
        exit(4);
    }

    clients_init();
    friend_list_init();
    
    while(running == 1){
        memset(message, 0, MAX_MESSAGE_SIZE);
        if(mq_receive(server_queue_id, message, MAX_MESSAGE_SIZE, NULL) == -1){
            if(errno != EINTR){
                perror("Unable to receive message.");
            }
        }
        else{
            printf("ODEBRANO %s \n", message);
            fetch_request();
            debug_printf_clients();
        }
    }

    for(int i =0; i < CLIENTS_SIZE; ++i){
        if(clients[i] != -1){
            send_response(STOP, i, "OK");
        }
    }

    for(int i = 0; i < CLIENTS_SIZE;++i){
        mq_close(clients[i]);
        remove_client(i);
    }

    return 0;
}