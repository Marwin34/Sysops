#define CLIENTS_SIZE 20
#define MSG_SIZE 256

#define MAX_MESSAGE_SIZE 512
#define MAX_MESSAGE_COUNT 10

//communication types
#define STOP 10
#define LIST 11
#define FRIENDS 12

#define INIT 13
#define ECHO 14
#define _2ALL 15
#define _2FRIENDS 16
#define _2ONE 17
#define ADD 18
#define DEL 19

#define COMMANDLINE_MODE 474
#define FILE_MODE 747


struct message
{
    int id;
    int argc;
    int argv[20];
    char text[256];
};

struct msgbuf
{
    long mtype; 
    struct message value;  
};

int string_to_type(char* data){
    if(strcmp(data, "STOP") == 0){
        return STOP;
    }
    else if(strcmp(data, "LIST") == 0){
        return LIST;
    }
    else if(strcmp(data, "FRIENDS") == 0){
        return FRIENDS;
    }
    else if(strcmp(data, "INIT") == 0){
        return INIT;
    }
    else if(strcmp(data, "ECHO") == 0){
        return ECHO;
    }
    else if(strcmp(data, "_2ALL") == 0){
        return _2ALL;
    }
    else if(strcmp(data, "_2FRIENDS") == 0){
        return _2FRIENDS;
    }
    else if(strcmp(data, "_2ONE") == 0){
        return _2ONE;
    }
    else if(strcmp(data, "ADD") == 0){
        return ADD;
    }
    else if(strcmp(data, "DEL") == 0){
        return DEL;
    }
    return -1;
}