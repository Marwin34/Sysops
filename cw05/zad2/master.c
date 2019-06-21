#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>


static int running = 1;

void handler(int s){
    running = 0;
    printf("\n");
}

int main(int argc, char** argv){

    signal(SIGINT, handler);

    if(argc != 2){
        printf("Niepoprawna liczba argumentow. \n");
    }

    char* path = argv[1];

    if (access(path, F_OK) != -1 && unlink(path) < 0) {
        printf("Nie mozna od usunac fifo. \n");
        exit(1);
    }

    if(mkfifo(path, 0644) == -1){
        perror(path);
        exit(1);
    }

    int fifo = open(path, O_RDONLY);

    if(fifo == -1){
        printf("Nie mozna bylo otworzyc fifo. \n");
    }

    while(running == 1){
        char line[4096];
        int read_value = read(fifo, line, sizeof(line));
        if( read_value == -1){
            printf("Blad podczas odczytywania fifo. \n");
            break;
        }else if(read_value > 0){
            printf("%s", line);
        }
    }

    close(fifo);

    if (unlink(path) < 0) {
        printf("Nie mozna usunac fifo. \n");
        exit(1);
    }
}