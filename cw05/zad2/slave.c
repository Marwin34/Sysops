#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>


int main(int argc, char** argv){
    if(argc != 3){
        printf("Niepoprawna liczba argumentów. \n");
        exit(1);
    }

    char* path = argv[1];
    char* end;
    long n = strtol(argv[2], &end, 10);

    if(end == argv[2] || errno != 0){
        printf("Niepoprawna wartosc n. \n");
        exit(1);
    }

    printf("%d rozpoczyna dzialanie! \n", getpid());

    struct stat sb;
    if (stat(path, &sb) == -1) {
        perror(path);
        exit(1);
    }

    if (!S_ISFIFO(sb.st_mode)) {
        printf("%s nie jest fifo. \n", path);
        exit(1);
    }

    int fifo = open(path, O_WRONLY);

    if(fifo == -1){
        perror(path);
        exit(1);    
    }

    time_t tt;
    int seed = time(&tt);
    srand(seed);

    time_t start_time;
    time(&start_time);

    time_t current_time = start_time;

    long long last_update = (long long)(current_time - start_time);

    int interval = (rand() % (5 - 2)) + 2;

    char date[512];
    char buffer[4096];
    while(n > 0){
        if((long long)(current_time - start_time) - last_update >= interval){
            last_update = (long long)(current_time - start_time);
            interval = (rand() % (5 - 2)) + 2;

            FILE* fdate = popen("date", "r");
            if (fgets(date, sizeof(date), fdate) == NULL) {
                printf("Nie mozna odczytac daty. \n");
                break;
            }   
            pclose(fdate);

            sprintf(buffer, "%d %s", getpid(), date);

            if(write(fifo, buffer, sizeof(buffer)) == -1){
                printf("Nie mozna bylo zapisac do fifo. \n");
                break;
            }

            --n;
        }
        time(&current_time);
    }
    close(fifo);
}