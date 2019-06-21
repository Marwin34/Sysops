#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/un.h>


#define TASK_LIST_LENGTH 20
#define MAX_CLIENTS_SIZE 10
#define MAX_NAME_LENGTH 255

#define FREE 0
#define BUSY 1

#define UNIX 2
#define INET 3

#define MAX_MESSAGE_SIZE 65507

pthread_mutex_t client_list_mutex = PTHREAD_MUTEX_INITIALIZER;

int port;
char* path;

int command_set = 0;

int server_socket_in = -1;
int server_socket_un = -1;

int running = 1;

struct client{
    int active;
    int status;
    int type;
    char* name;
    struct sockaddr* addr;
    socklen_t addr_len;
    int tasks[TASK_LIST_LENGTH];
};

struct client clients[MAX_CLIENTS_SIZE];

int send_resp_in(struct sockaddr* addr, socklen_t addr_len, int id, char* data){
    if(sendto(server_socket_in, &id, sizeof(id), 0, addr, addr_len) == -1){
        perror("ERROR SEND! \n");
        return -1;
    }

    int len = strlen(data);

    if(sendto(server_socket_in, &len, sizeof(len), 0, addr, addr_len) == -1){
        perror("ERROR SEND! \n");
        return -1;
    }

    char* message = calloc(strlen(data), sizeof(char));

    sprintf(message, "%s", data);

    if(sendto(server_socket_in, message, MAX_MESSAGE_SIZE, 0, addr, addr_len) == -1){
        perror("ERROR SEND! \n");
        return -1;
    }

    free(message);

    return 1;
}

int send_resp_un(struct sockaddr* addr, socklen_t addr_len, int id, char* data){
    if(sendto(server_socket_un, &id, sizeof(id), 0, addr, addr_len) == -1){
        perror("ERROR SEND! \n");
        return -1;
    }

    int len = strlen(data);

    if(sendto(server_socket_un, &len, sizeof(len), 0, addr, addr_len) == -1){
        perror("ERROR SEND! \n");
        return -1;
    }

    char* message = calloc(strlen(data), sizeof(char));

    sprintf(message, "%s", data);

    if(sendto(server_socket_un, message, MAX_MESSAGE_SIZE, 0, addr, addr_len) == -1){
        perror("ERROR SEND! \n");
        return -1;
    }

    free(message);

    return 1;
}

int unique_name(char* name){    
    for(int i = 0; i < MAX_CLIENTS_SIZE; ++i){
        if(strcmp(clients[i].name, name) == 0){
            return -1;
        }
    } 
    return 1;
}

int client_size(){
    int cnt = 0;

    for(int i = 0; i < MAX_CLIENTS_SIZE; ++i){
        if(clients[i].active == 1){
            ++cnt;
        }
    }

    return cnt;
}

int client_index(struct sockaddr* addr,socklen_t addr_len) {
    for(int i = 0; i < MAX_CLIENTS_SIZE; ++i){
        if(clients[i].addr_len == addr_len &&
            memcmp(addr, clients[i].addr, addr_len) == 0)
         return i;
    }
    return -1;
}

void add_to_client_tasks(int index, int task){
    for(int i = 0; i < TASK_LIST_LENGTH; ++i){
        if(clients[index].tasks[i] == -1){
            clients[index].tasks[i] = task;
            break;
        }
    }
}

int clients_tasks(int index){
    int cnt = 0;

    for(int i = 0; i < TASK_LIST_LENGTH; ++i){
        if(clients[index].tasks[i] > 0){
            ++cnt;
        }
    }
    return cnt;
}

void add_client(char* name, struct sockaddr* addr, socklen_t addr_len, int type){
    for(int i = 0; i < MAX_CLIENTS_SIZE; ++i){
        if(clients[i].active == 0){
            clients[i].active = 1;
            clients[i].status = FREE;
            clients[i].type = type;
            clients[i].addr = addr;
            clients[i].addr_len = addr_len;
            memset(clients[i].name, 0, MAX_NAME_LENGTH);
            strcpy(clients[i].name, name);

            for(int j = 0; j < TASK_LIST_LENGTH; ++j){
                clients[i].tasks[j] = -1;
            }

            break;
        }
    }
}

void remove_from_client_tasks(int index, int task){

    if(index == -1){
        return;
    }

    for(int i = 0; i < TASK_LIST_LENGTH; ++i){
        if(clients[index].tasks[i] == task){
            clients[index].tasks[i] = -1;
            break;
        }
    }
}

void remove_client(int id){

    for(int i = id; i < MAX_CLIENTS_SIZE - 1; ++i){
        clients[i] = clients[i + 1];
    }

    clients[MAX_CLIENTS_SIZE - 1].active = 0;
    memset(clients[MAX_CLIENTS_SIZE - 1].name, 0, MAX_NAME_LENGTH);   
}

void clean() {
    for(int i = 0; i < MAX_CLIENTS_SIZE; ++i) {
        if(clients[i].active == 1){
            if(clients[i].type == UNIX){
                send_resp_un(clients[i].addr, clients[i].addr_len, -5, "QUIT");
            }else{
                send_resp_in(clients[i].addr, clients[i].addr_len, -5, "QUIT");
            }
        }
    }

    for(int i = 0; i < MAX_CLIENTS_SIZE; ++i) {
        if(clients[i].active == 1){
            remove_client(i);
        }
    }

    unlink(path);

    if(server_socket_in != -1) {
        close(server_socket_in);
    }
    if(server_socket_un != -1) {
        close(server_socket_un);
    }
}

void signal_handler() {
    exit(0);
}

void* input_routine(void* arg){
    int task_id = 1;

    while(running == 1){
        char command[256]; 

        char letter;
        int position = 0;

        while (running == 1 && (letter = fgetc(stdin)) != '\n') {
            command[position++] = letter;
        }
        command[position] = '\0';

        FILE *f = fopen(command, "rb");
        if(f == NULL){
            printf("Unable to open file %s. \n", command);
        }else{
            fseek(f, 0, SEEK_END);
            long fsize = ftell(f);
            fseek(f, 0, SEEK_SET);

            char *file_data = malloc(fsize + 1);
            fread(file_data, 1, fsize, f);
            fclose(f);

            file_data[fsize] = 0;

            if(client_size() > 0){  
                pthread_mutex_lock(&client_list_mutex);
                int sended = 0;

                for(int i = 0; i < MAX_CLIENTS_SIZE; ++i){
                    if(clients[i].active == 1){
                        if(clients_tasks(i) > 0){
                            clients[i].status = BUSY;
                        }else{
                            clients[i].status = FREE;
                        }
                    }
                }

                for(int i = 0; i < MAX_CLIENTS_SIZE; ++i){
                    if(clients[i].active == 1 && clients[i].status == FREE){
                        add_to_client_tasks(i, task_id);
                        if(clients[i].type == INET){
                            send_resp_in(clients[i].addr, clients[i].addr_len, task_id, file_data);
                        }else{
                            send_resp_un(clients[i].addr, clients[i].addr_len, task_id, file_data);
                        }
                        sended = 1;
                        break;
                    }
                }

                if(sended == 0){
                    int worker = rand() % client_size();
                        if(clients[worker].type == INET){
                            send_resp_in(clients[worker].addr, clients[worker].addr_len, task_id, file_data);
                        }else{
                            send_resp_un(clients[worker].addr, clients[worker].addr_len, task_id, file_data);
                        }
                }

                ++task_id;        

                pthread_mutex_unlock(&client_list_mutex);
                free(file_data);
            } else{
                printf("No cients connected. \n");
            }
        }
    }

    return NULL;
}

void* comm_routine(void* arg){ 
    int activity;
    int max_sd;   
         
    fd_set readfds;     

    puts("Waiting for connections ... \n");   
         
    while(running == 1) {   

        FD_ZERO(&readfds);   
     
        FD_SET(server_socket_in, &readfds);   
        FD_SET(server_socket_un, &readfds);
        max_sd = server_socket_in;  

        if(max_sd < server_socket_un){
            max_sd = server_socket_un;
        }
        activity = select(max_sd + 1 , &readfds , NULL , NULL , NULL);   
        if ((activity < 0) && (errno!=EINTR)){   
            printf("select error");   
        }else if(activity > 0){
            pthread_mutex_lock(&client_list_mutex);
            if (FD_ISSET(server_socket_in, &readfds)){  
                int id = -9;
                int len = 0; 

                struct sockaddr* from_addr = malloc(sizeof(struct sockaddr_in));
                socklen_t from_addr_len = sizeof(struct sockaddr_in);

                recvfrom(server_socket_in, &id, sizeof(int), MSG_WAITALL, from_addr, &from_addr_len);

                recvfrom(server_socket_in, &len, sizeof(int), MSG_WAITALL, from_addr, &from_addr_len);

                char* buf = calloc(len, sizeof(char));

                recvfrom(server_socket_in, buf, len, MSG_WAITALL, from_addr, &from_addr_len);

                if(id == -1){
                    if(unique_name(buf) == -1){
                        send_resp_in(from_addr, from_addr_len, -1, "REJECTED");
                    }else if(unique_name(buf) == 1){
                        add_client(buf, from_addr, from_addr_len, INET);
                        send_resp_in(from_addr, from_addr_len, -2, "ACCEPTED");
                    }
                }else if(id >= 0){
                    printf("Received result from client %d for task %d: %s \n", client_index(from_addr, from_addr_len), id, buf);
                    remove_from_client_tasks(client_index(from_addr, from_addr_len), id);
                }else if(id == -5){
                    send_resp_in(from_addr, from_addr_len, -5, "QUIT");
                    remove_client(client_index(from_addr, from_addr_len));
                }
                free(buf);
            }  
            if (FD_ISSET(server_socket_un, &readfds)){   
                int id = -9;
                int len = 0; 

                struct sockaddr* from_addr = malloc(sizeof(struct sockaddr_un));
                socklen_t from_addr_len = sizeof(struct sockaddr_un);

                recvfrom(server_socket_un, &id, sizeof(int), MSG_WAITALL, from_addr, &from_addr_len);

                recvfrom(server_socket_un, &len, sizeof(int), MSG_WAITALL, from_addr, &from_addr_len);

                char* buf = calloc(len, sizeof(char));

                recvfrom(server_socket_un, buf, len, MSG_WAITALL, from_addr, &from_addr_len);

                if(id == -1){
                    if(unique_name(buf) == -1){
                        send_resp_un(from_addr, from_addr_len, -1, "REJECTED");
                    }else if(unique_name(buf) == 1){
                        add_client(buf, from_addr, from_addr_len, UNIX);
                        send_resp_un(from_addr, from_addr_len, -2, "ACCEPTED");
                    }
                }else if(id >= 0){
                    printf("Received result from client %d for task %d: %s \n", client_index(from_addr, from_addr_len), id, buf);
                    remove_from_client_tasks(client_index(from_addr, from_addr_len), id);
                }else if(id == -5){
                    send_resp_un(from_addr, from_addr_len, -5, "QUIT");
                    remove_client(client_index(from_addr, from_addr_len));
                }
                free(buf);
            }
            pthread_mutex_unlock(&client_list_mutex);
        } 
    }   
         
    return NULL;
}

int main(int argc, char** argv){
    atexit(clean);
    signal(SIGINT, signal_handler);

    if(argc != 3){
        printf("Argument count missmatch!");
        exit(1);
    }

    if(sscanf(argv[1], "%d", &port) != 1){
        printf("Badd port. \n");
        exit(2);
    }

    path = argv[2];

    struct sockaddr_in addr_in;
    addr_in.sin_family = AF_INET;   
    addr_in.sin_addr.s_addr = INADDR_ANY;
    addr_in.sin_port = htons( port );   

    if((server_socket_in = socket(AF_INET , SOCK_DGRAM , 0)) == 0)   
    {   
        perror("socket failed");   
        exit(1);   
    }

    if(bind(server_socket_in, (struct sockaddr *)&addr_in, sizeof(addr_in)) < 0)   
    {   
        perror("bind failed");   
        exit(1);   
    }

    server_socket_un = socket(AF_UNIX, SOCK_DGRAM, 0);

    if (server_socket_un == -1) {
        perror("unable to create network socket");
        exit(1);
    }

    int op = 1;
    setsockopt(server_socket_un, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(int));

    struct sockaddr_un name;
    name.sun_family = AF_UNIX;
    strcpy(name.sun_path, path);

    unlink(path);
    if(bind(server_socket_un, (struct sockaddr*)&name, sizeof(name)) < 0) {
        perror("unable to bind unix");
        exit(1);
    }

    for (int i = 0; i < MAX_CLIENTS_SIZE; i++){   
        clients[i].active = 0;
        clients[i].name = malloc(MAX_NAME_LENGTH * sizeof(char));
    }      

    int seed;
    time_t tt;
    seed = time(&tt);
    srand(seed);

    pthread_t threads_key[2];
    if(pthread_create(&threads_key[0], NULL, input_routine, NULL) != 0){
        perror("Unable to create input thread \n");
        exit(1);
    }

    if(pthread_create(&threads_key[1], NULL, comm_routine, NULL) != 0){
        perror("Unable to create comm thread \n");
        exit(2);
    }

    pthread_join(threads_key[0], NULL);
    printf("Ended \n");
    pthread_join(threads_key[1], NULL);
    printf("Ended \n");

    close(server_socket_in);
    close(server_socket_un);

    return 0;
}