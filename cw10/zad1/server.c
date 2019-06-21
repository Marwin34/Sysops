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

pthread_mutex_t client_list_mutex = PTHREAD_MUTEX_INITIALIZER;

int port;
char* path;

int command_set = 0;

int server_socket_in;
int server_socket_un;

struct client{
    int active;
    int socket;
    int status;
    char* name;
    int tasks[TASK_LIST_LENGTH];
};

struct client clients[MAX_CLIENTS_SIZE];


int send_resp(int sd, int id, char* data){
    if(send(sd, &id, sizeof(int), 0) != sizeof(int)){
        printf("ERROR SEND! \n");
        return -1;
    }

    int len = strlen(data);

    if(send(sd, &len, sizeof(int), 0) != sizeof(int)){
        printf("ERROR SEND! \n");
        return -1;
    }

    char* message = calloc(strlen(data), sizeof(char));

    sprintf(message, "%s", data);

    if(send(sd, message, strlen(message), 0) != strlen(message)){
        printf("ERROR SEND! \n");
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
        if(clients[i].active > 0){
            ++cnt;
        }
    }

    return cnt;
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

void remove_from_client_tasks(int index, int task){
    for(int i = 0; i < TASK_LIST_LENGTH; ++i){
        if(clients[index].tasks[i] == task){
            clients[index].tasks[i] = -1;
            break;
        }
    }
}

void register_socket(int socket){
    for(int i = 0; i < MAX_CLIENTS_SIZE; ++i){
        if(clients[i].socket == 0 && clients[i].active == 0){
            clients[i].socket = socket;

            break;
        }
    }
}

void unregister_socket(int socket){
    for(int i = 0; i < MAX_CLIENTS_SIZE; ++i){
        if(clients[i].socket == socket){
            clients[i].socket = 0;
            break;
        }
    }
}

void add_client(char* name, int socket){
    for(int i = 0; i < MAX_CLIENTS_SIZE; ++i){
        if(clients[i].active == 0 && clients[i].socket == socket){
            clients[i].active = 1;
            clients[i].status = FREE;
            memset(clients[i].name, 0, MAX_CLIENTS_SIZE);
            strcpy(clients[i].name, name);

            break;
        }
    }
}

void remove_client(int id){
    close(clients[id].socket);

    for(int i = id; i < MAX_CLIENTS_SIZE - 1; ++i){
        clients[i] = clients[i + 1];
    }

    clients[MAX_CLIENTS_SIZE - 1].active = 0;
    clients[MAX_CLIENTS_SIZE - 1].socket = 0; 
    clients[MAX_CLIENTS_SIZE - 1].status = FREE;

    memset(clients[MAX_CLIENTS_SIZE - 1].name, 0, MAX_NAME_LENGTH);

    for(int i = 0; i < TASK_LIST_LENGTH; ++i){
        clients[MAX_CLIENTS_SIZE - 1].tasks[i] = -1;
    }
}

void clean() {
    for(int i = 0; i < MAX_CLIENTS_SIZE; ++i) {
        if(clients[i].active == 1){
            send_resp(clients[i].socket, -5, "QUIT");
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

    while(1){
        char command[256]; 

        char letter;
        int position = 0;

        while ((letter = fgetc(stdin)) != '\n') {
            command[position++] = letter;
        }
        command[position] = '\0';

        FILE *f = fopen(command, "rb");
        if(f == NULL){
            printf("Unable to open file %s. \n", command);
        }else{
            fseek(f, 0, SEEK_END);
            long fsize = ftell(f);
            fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

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
                        send_resp(clients[i].socket, task_id, file_data);
                        sended = 1;
                        break;
                    }
                }

                if(sended == 0){
                    int worker = rand() % client_size();
                    send_resp(clients[worker].socket, task_id, file_data);
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
    int new_socket, activity, i, sd;   
    int max_sd;   
         
    fd_set readfds;     

    puts("Waiting for connections ... \n");   
         
    while(1) {   

        FD_ZERO(&readfds);   
     
        FD_SET(server_socket_in, &readfds);   
        FD_SET(server_socket_un, &readfds);
        max_sd = server_socket_in;  

        if(max_sd < server_socket_un){
            max_sd = server_socket_un;
        } 
              
        pthread_mutex_lock(&client_list_mutex);
        for ( i = 0 ; i < MAX_CLIENTS_SIZE ; i++)   
        {  
            sd = clients[i].socket;   
                  
            if(sd > 0)   
                FD_SET(sd, &readfds);   
                  
            if(sd > max_sd)   
                max_sd = sd;   
        }   
        pthread_mutex_unlock(&client_list_mutex);

        activity = select(max_sd + 1 , &readfds , NULL , NULL , NULL);   
        if ((activity < 0) && (errno!=EINTR)){   
            printf("select error");   
        }else if(activity > 0){
            pthread_mutex_lock(&client_list_mutex);
            if (FD_ISSET(server_socket_in, &readfds)){   
                if((new_socket = accept(server_socket_in, NULL, NULL)) == -1) {   
                    perror("accept");   
                    exit(EXIT_FAILURE);   
                }

                register_socket(new_socket);   
            }  
            if (FD_ISSET(server_socket_un, &readfds)){   
                if((new_socket = accept(server_socket_un, NULL, NULL)) == -1) {   
                    perror("accept");   
                    exit(EXIT_FAILURE);   
                }

                register_socket(new_socket);   
            }   
            for (i = 0; i < MAX_CLIENTS_SIZE; i++)   
            {   
                sd = clients[i].socket;   
                
                if (FD_ISSET(sd, &readfds))   
                {   
                    int id = -9;
                    int len = 0;
                    
                    recv(sd, &id, sizeof(int), MSG_WAITALL);

                    recv(sd, &len, sizeof(int), MSG_WAITALL);
                    
                    char* buf = calloc(len, sizeof(char));

                    recv(sd, buf, len, MSG_WAITALL);
                    if(id == -1){
                        if(unique_name(buf) == -1){
                            send_resp(sd, -1, "REJECTED");
                            unregister_socket(sd);
                        }else if(unique_name(buf) == 1){
                            add_client(buf, sd);
                            send_resp(sd, -2, "ACCEPTED");
                        }
                    }else if(id >= 0){
                        printf("Received result from client %d for task %d: %s \n", i, id, buf);
                        remove_from_client_tasks(i, id);
                    }else if(id == -5){
                        send_resp(sd, -5, "QUIT");
                        remove_client(i);
                    }
                }   
            }  
            pthread_mutex_unlock(&client_list_mutex);
        } 
    }   
         
    return NULL;
}

int main(int argc, char** argv){

    signal(SIGINT, signal_handler);

    atexit(clean);

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

    if( (server_socket_in = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
    {   
        perror("socket failed");   
        exit(1);   
    }

    if (bind(server_socket_in, (struct sockaddr *)&addr_in, sizeof(addr_in)) < 0)   
    {   
        perror("bind failed");   
        exit(1);   
    }   
         
    if (listen(server_socket_in, 3) < 0)   
    {   
        perror("listen");   
        exit(1);   
    }   

    struct sockaddr_un addr_un;
    addr_un.sun_family = AF_UNIX;
    strcpy(addr_un.sun_path, path);
    unlink(addr_un.sun_path);

    if( (server_socket_un = socket(AF_UNIX , SOCK_STREAM , 0)) == 0)   
    {   
        perror("socket failed");   
        exit(1);   
    }

    if (bind(server_socket_un, (struct sockaddr*)&addr_un, sizeof(addr_un)) == -1) {
        perror("unix bind failed");
        exit(1);
    }

    if (listen(server_socket_un, 5) == -1) {
        perror("unable to listen on unix socket");
        exit(1);
    }
          
    for (int i = 0; i < MAX_CLIENTS_SIZE; i++){   
        clients[i].active = 0;
        clients[i].socket = 0;   
        clients[i].status = FREE;
        clients[i].name = malloc(MAX_CLIENTS_SIZE * sizeof(char));

        for(int j = 0; j < TASK_LIST_LENGTH; ++j){
            clients[i].tasks[j] = -1;
        }
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
    
    pthread_join(threads_key[1], NULL);
    printf("Ended \n");

    close(server_socket_in);
    close(server_socket_un);

    return 0;
}