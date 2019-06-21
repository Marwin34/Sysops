#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>


int main(int argc, char** argv){
    int seed;
    time_t tt;
    seed = time(&tt);
    srand(seed);

    if(argc == 5){
        char* target = argv[1];
        int pmin = atoi(argv[2]);
        int pmax = atoi(argv[3]);
        int bytes = atoi(argv[4]);

        int interval = (rand() % (pmax - pmin)) + pmin;

        int life_span = 30;

        pid_t pid = getpid();

        time_t now;
        struct tm* timeinfo;
        timeinfo = localtime(&now);

        char date[80];
        strftime(date, sizeof(date), "%Y-%m-%d_%H-%M-%S", timeinfo);

        char* rest = calloc(bytes, sizeof(char*));
        FILE* fp = fopen("/dev/urandom", "rb");
        fread(rest, bytes, 1, fp);
        fclose(fp);

        time_t start_time;
        time(&start_time);

        time_t current_time = start_time;

        long long last_update = (long long)(current_time - start_time);

        while((long long)(current_time - start_time) <= life_span){
            if((long long)(current_time - start_time) - last_update >= interval){
                last_update = (long long)(current_time - start_time);

                FILE* fp = fopen(target, "a");

                if(fp == NULL){
                    printf("Nie mozna otworzyc pliku %s.", target);
                    return 1;
                }
            
                printf("Zapisuje do pliku %s. \n", target);

                fprintf(fp, "pid: %d sleep: %d date: %s\nbytes: ",
                    pid, interval, date);

                fwrite(rest, bytes, 1, fp);
                fputs("\n\n", fp);
                fclose(fp);

            }
            time(&current_time);
        }
        free(rest);
    }

    return 0;
}