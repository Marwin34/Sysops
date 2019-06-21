#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

#define MAX_TASKS 3


pthread_mutex_t task_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t task_list_cond = PTHREAD_COND_INITIALIZER;

int fd = -1; 
int running = 1;

char* task_list[MAX_TASKS];
int task_id[MAX_TASKS];

int count_distinct_words(char* data){
    int cnt = 0;

    char *tmp = strtok( data, " ,.-\n\t");
    while(tmp != NULL){
        if(strcmp(tmp, "—") != 0 && isspace(tmp[0]) == 0){
            ++cnt;
        }
        tmp = strtok(NULL, " ,.-\n\t");
    }
    sleep(2);
    return cnt;
}

void add_to_tasklist(char* task, int id){
    for(int i = 0; i < MAX_TASKS; ++i){
        if(task_id[i] == -1){
            task_id[i] = id;

            task_list[i] = malloc(strlen(task) * sizeof(char));
            strcpy(task_list[i], task);
            break;
        }
    }
}

void task_list_shift(){
    for(int i = 0; i < MAX_TASKS - 1; ++i){
        if(task_id[i] > 0){
            free(task_list[i]);
            if(task_id[i + 1] > 0){
                task_id[i] = task_id[i + 1];
                task_list[i] = malloc(strlen(task_list[i + 1]) * sizeof(char));
                strcpy(task_list[i], task_list[i + 1]);
            }else{
                task_id[i] = -1;
            }
        }
    }

    if(task_id[MAX_TASKS - 1] > 0){
        task_id[MAX_TASKS - 1] = -1;
        free(task_list[MAX_TASKS - 1]);
    }
}

void signal_handler(int sig){
    int id = -5;
    char* data = calloc(20, sizeof(char));
    sprintf(data, "%s", "QUIT");

    if(send(fd, &id, sizeof(int), 0) != sizeof(int)){
        printf("Error send! \n");
    }

    int len = strlen(data);

    if(send(fd, &len, sizeof(int), 0) != sizeof(len)){
        printf("Error send! \n");
    }

    if(send(fd, data, strlen(data), 0) != strlen(data)){
        printf("Error send! \n");
    }

    free(data);
}

void* receive_routine(void* arg){
    while(running == 1){
        int id = -9;
        int len = 0;

        recv(fd, &id, sizeof(int), MSG_WAITALL);

        recv(fd, &len, sizeof(int), MSG_WAITALL);

        char* buf = calloc(len, sizeof(char));

        recv(fd, buf, len, MSG_WAITALL);

        if(id == -3){
            printf("Sended badval. \n");
        }else if(id == -2){
            printf("Name accepted. \n");
        }else if(id == -1){
            printf("Name rejected. \n");
            running = 0;
            pthread_cond_broadcast(&task_list_cond);
        }else if(id == -5){
            running = 0;
            pthread_cond_broadcast(&task_list_cond);
        }else{
            printf("Received task %d. \n", id);
            pthread_mutex_lock(&task_list_mutex);
            add_to_tasklist(buf,id);
            pthread_cond_broadcast(&task_list_cond);
            pthread_mutex_unlock(&task_list_mutex);
        }

        if(id < 0){
            free(buf);
        }
    }

    return NULL;
}

void* task_routine(void* arg){
    while(running == 1){
        char* buf;
        int id = -1;
        pthread_mutex_lock(&task_list_mutex);
        while(task_id[0] == -1){
            if(running != 1){
                return NULL;
            }
            pthread_cond_wait(&task_list_cond, &task_list_mutex);
        }

        buf = malloc(strlen(task_list[0]) * sizeof(char));
        strcpy(buf, task_list[0]);
        id = task_id[0];
        task_list_shift();

        pthread_mutex_unlock(&task_list_mutex);

        if(id != -1){
            int result = count_distinct_words(buf);
            free(buf);

            char* data = calloc(20, sizeof(char));
            sprintf(data, "%d", result);

            if(send(fd, &id, sizeof(int), 0) != sizeof(int)){
                printf("Error send! \n");
                continue;
            }

            int len = strlen(data);

            if(send(fd, &len, sizeof(int), 0) != sizeof(len)){
                printf("Error send! \n");
                continue;
            }
            if(send(fd, data, strlen(data), 0) != strlen(data)){
                printf("Error send! \n");
                continue;
            }
            free(data);
        }
    }

    return NULL;
}

int main(int argc, char** argv){

    signal(SIGINT, signal_handler);

    if(argc != 4){
        printf("Arguments count missmatch. \n");
        exit(1);
    }

    char* name = argv[1];

    char* type = argv[2];

    char* port = argv[3];

    if(strcmp(type, "UNIX") == 0){
        fd = socket(AF_UNIX, SOCK_DGRAM, 0);
        if(fd == -1){
            perror("cannot make socket");
            exit(1);
        }

        int option = 1;
        if(setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &option, sizeof(int)) != 0){
            perror("Cannot set sockopt");
            exit(1);
        }

        struct sockaddr_un addr;
        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, argv[3]);

        if(bind(fd, (struct sockaddr*)&addr, sizeof(sa_family_t)) != 0){
            perror("Cannot bind");
            exit(1);
        }

        if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0){
            perror("Cannot connect");
            exit(1);
        } 

    }else if(strcmp(type, "INET") == 0){
        struct sockaddr_in addr;    

        if((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            perror("socket error");
            exit(1);
        }

        int port_i;
        if(sscanf(port, "%d", &port_i) != 1){
            printf("Unable to scanf port. \n");
            exit(2);
        }
        
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;    
        addr.sin_port = htons(port_i);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
            perror("connect error");
            exit(1);
        }
    }

    int id;
    int len;

    char* data = calloc(strlen(name), sizeof(char));
    sprintf(data, "%s", name);

    id = -1;

    if(send(fd, &id, sizeof(int), 0) != sizeof(int)){
        printf("Error send! \n");
        exit(2);
    }

    len = strlen(data);

    if(send(fd, &len, sizeof(int), 0) != sizeof(int)){
        printf("Error send! \n");
        exit(2);
    }

    if(send(fd, data, strlen(data), 0) != strlen(data)){
        printf("Error send! \n");
        exit(2);
    }

    free(data);

    for(int i = 0; i < MAX_TASKS; ++i){
        task_id[i] = -1;
    }

    pthread_t threads_key[2];
    if(pthread_create(&threads_key[0], NULL, receive_routine, NULL) != 0){
        perror("Unable to create receive thread \n");
        exit(1);
    }

    if(pthread_create(&threads_key[1], NULL, task_routine, NULL) != 0){
        perror("Unable to create task thread \n");
        exit(2);
    }

    pthread_join(threads_key[0], NULL);
    
    pthread_join(threads_key[1], NULL);

    close(fd);

    return 0;
}