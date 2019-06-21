#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <unistd.h>
#include <limits.h>
#include <dlfcn.h>

static clock_t st_time;
static clock_t en_time;
static struct tms st_cpu;
static struct tms en_cpu;
 
void start_clock() {
    st_time = times(&st_cpu);
}
 
void stop_clock(char** buffer) {
    en_time = times(&en_cpu);
    int64_t clk_tck = sysconf(_SC_CLK_TCK);

    *buffer = calloc(strlen("Real: 000.000, User: 000.000, System: 000.000\n") + 1, sizeof(char*));

    double real_time = (double)(en_time - st_time) / clk_tck;

    double user_time = (double)(en_cpu.tms_utime - st_cpu.tms_utime) / clk_tck 
                        + (double)(en_cpu.tms_cutime - st_cpu.tms_cutime) / clk_tck;

    double system_time = (double)(en_cpu.tms_stime - st_cpu.tms_stime) / clk_tck 
                        + (double)(en_cpu.tms_cstime - st_cpu.tms_cstime) / clk_tck;

    sprintf(*buffer, "Real: %.3fs, User: %.3fs, System: %.3fs\n", real_time, user_time, system_time);
}

void log_buffer(char* buffer){
    if(strlen(buffer) > 0){
        printf(buffer);

        FILE* fp;

        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            char* log_file_path = calloc(strlen(cwd) + 5, sizeof(char*));

            strcpy(log_file_path, cwd);
            strcat(log_file_path, "/log");

            fp = fopen(log_file_path, "a");

            fputs(buffer, fp);

            fclose(fp);

            free(log_file_path);
            free(buffer);
            buffer = NULL;

        } else {
            free(buffer);
            buffer = NULL;
            perror("getcwd() error");
            return;
        }
    }
}

int main(int argc, char* argv[])
{   
    void *handle = dlopen("libfind_library.so", RTLD_LAZY);

    if(!handle){
        printf("Couldn't open library.");
        return -1;
    }

    void (*create_table)();
    create_table = (void (*)())dlsym(handle, "create_table");

    if(dlerror() != NULL){
        printf("Couldn't bind create_table function.");
        return -1;
    }


    void (*set_targets)(char dir_path[], char file_name[]);
    set_targets = (void (*)(char dir_path[], char file_name[]))dlsym(handle, "set_targets");

    if(dlerror() != NULL){
        printf("Couldn't bind set_targets function.");
        return -1;
    }

    void (*search_directory)(char tmp_file_name[]);
    search_directory = (void (*)(char tmp_file_name[]))dlsym(handle, "search_directory");

    if(dlerror() != NULL){
        printf("Couldn't bind search_directory function.");
        return -1;
    }

    int (*load_to_memory)();
    load_to_memory = (int (*)())dlsym(handle, "load_to_memory");

    if(dlerror() != NULL){
        printf("Couldn't bind load_to_memory function.");
        return -1;
    }

    void (*remove_block)(int index);
    remove_block = (void (*)(int index))dlsym(handle, "remove_block");

    if(dlerror() != NULL){
        printf("Couldn't bind remove_block function.");
        return -1;
    }

    void (*free_table)();
    free_table = (void (*)())dlsym(handle, "free_table");

    if(dlerror() != NULL){
        printf("Couldn't bind free_table function.");
        return -1;
    }

    char* buffer = NULL;

    for(int i = 1; i < argc; ++i){
        if(strcmp(argv[i], "create_table") == 0){
            int size = atoi(argv[i + 1]);

            if(size < 0){
                printf("Wrong size attribute: %d", size);
                continue;
            }

            create_table(size);

            ++i;
        }else if(strcmp(argv[i], "search_directory") == 0){
            char* searching_dir = argv[i + 1];
            char* searching_file = argv[i + 2];
            char* result_file = argv[i + 3];

            set_targets(searching_dir, searching_file);
            search_directory(result_file);

            i += 3;
        }else if(strcmp(argv[i], "remove_block") == 0){
            int index = atoi(argv[i + 1]);

            remove_block(index);

            ++i;
        }else if(strcmp(argv[i], "load_to_memory") == 0){
            
            load_to_memory();

        }else if(strcmp(argv[i], "log") == 0){
            log_buffer(buffer);
        }else if(strcmp(argv[i], "start_clock") == 0){
            start_clock();
        }else if(strcmp(argv[i], "stop_clock") == 0){
            stop_clock(&buffer);
        }
    }

    free_table();

    dlclose(handle);

    return 0;
}