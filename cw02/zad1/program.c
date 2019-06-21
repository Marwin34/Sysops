#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <limits.h>

static clock_t st_time;
static clock_t en_time;
static struct tms st_cpu;
static struct tms en_cpu;

/*
    FUNCKJE DO POMIARU CZASU ORAZ ZAPISU WYNIKU PMIARU W LOGU
*/
 
void start_clock() {
    st_time = times(&st_cpu);
}
 
void stop_clock(char** buffer) {
    en_time = times(&en_cpu);
    int64_t clk_tck = sysconf(_SC_CLK_TCK);

    *buffer = calloc(strlen("User: 000.000, System: 000.000") + 1, sizeof(char*));

    double user_time = (double)(en_cpu.tms_utime - st_cpu.tms_utime) / clk_tck;

    double system_time = (double)(en_cpu.tms_stime - st_cpu.tms_stime) / clk_tck;;

    sprintf(*buffer, "User: %.3fs, System: %.3fs", user_time, system_time);
}

void log_buffer(char* buffer, char* msg){
    if(strlen(buffer) > 0){
        printf("%s\n", msg);
        printf("%s\n", buffer);

        FILE* fp;

        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            char* log_file_path = calloc(strlen(cwd) + 5, sizeof(char*));

            strcpy(log_file_path, cwd);
            strcat(log_file_path, "/wyniki.txt");

            fp = fopen(log_file_path, "a");

            if (fp == NULL) {
                perror(log_file_path);
                return;
            }            

            fprintf(fp, "%s\n", msg);
            fprintf(fp, "%s\n", buffer);

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

/*
    WLASCIWE FUNKCJE PROGRAMU
*/

void generate(char* name, int size, int bytes){
    char actual_size_string[50];

    sprintf(actual_size_string, "%d", size * bytes);

    char* command = calloc(strlen("head -c  /dev/urandom > ") + strlen(actual_size_string) + strlen(name) + 1 , sizeof(char*));
    
    sprintf(command, "head -c %d /dev/urandom > %s", size * bytes, name);

    system(command);
}

int lseek_then_read(int fd, int pos, int bytes, char* buf){
    if(lseek(fd, pos * bytes, SEEK_SET) == -1){
        perror("Blad podczas przemieszczania sie po pliku. ");
        return 0;
    }
    if(read(fd, buf, bytes) == -1){
        perror("Blad podczas wczytywania zawartosci pliku. ");
        return 0;
    }
    return 1;
}

int lseek_the_write(int fd, int pos, int bytes, char* buf){
    if(lseek(fd, pos * bytes, SEEK_SET) == -1){
        perror("Blad podczas przemieszczania sie po pliku. ");
        return 0;
    }
    if(write(fd, buf, bytes) == -1){
        perror("Blad podczas wczytywania zawartosci pliku. ");
        return 0;
    }
    return 1;
}

int fseek_then_freed(FILE* fp, int pos, int bytes, char* buf){
    if(fseek(fp, pos * bytes, 0) != 0){
        perror("Blad podczas przemieszczania sie po pliku. \n");
        return 0;
    }
    fread(buf, bytes, 1, fp);
    if(ferror(fp) != 0){
        perror("Blad podczas odczytywania pliku. \n");
        return 0;
    }

    return 1;
}

int fseek_then_fwrite(FILE* fp, int pos, int bytes, char* buf){
    if(fseek(fp, pos * bytes, 0) != 0){
        perror("Blad podczas przemieszczania sie po pliku. \n");
        return 0;
    }
    fwrite(buf, bytes, 1, fp);
    if(ferror(fp) != 0){
        perror("Blad podczas odczytywania pliku. \n");
        return 0;
    }
    return 1;
}

int sort_sys(char* name, int size, int bytes){
    int fd = open(name, O_RDWR);

    char* buf1 = calloc(bytes, sizeof(char*));
    char* buf2 = calloc(bytes, sizeof(char*));

    for(int i = 0; i < size; ++i){
        int min = i;

        if(lseek_then_read(fd, min, 1, buf1) == 0){
            return 0;
        }

        for(int j = i + 1; j < size; ++j){
            if(lseek_then_read(fd, j, 1, buf2) == 0){
                return 0;
            }

            if(buf1[0] > buf2[0]){
                min = j;

                if(lseek_then_read(fd, min, 1, buf1) == 0){
                    return 0;
                }
            }
        }

        if(min != i){
            if(lseek_then_read(fd, i, bytes, buf1) == 0){
                return 0;
            }
            if(lseek_then_read(fd, min, bytes, buf2) == 0){
                return 0;
            }

            if(lseek_the_write(fd, i, bytes, buf2) == 0){
                return 0;
            }
            if(lseek_the_write(fd, min, bytes, buf1) == 0){
                return 0;
            }
        }
    }   
    close(fd);

    free(buf1);
    free(buf2);

    return 1;
}

int copy_sys(char* name1, char* name2, int size, int bytes){
    int fd1 = open(name1, O_RDONLY);
    int fd2 = open(name2, O_WRONLY | O_CREAT, 0666);

    char* buf = calloc(bytes, sizeof(char*));

    for(int i = 0; i < size; ++i){
        read(fd1, buf, bytes);
        write(fd2, buf, bytes);
    }

    close(fd1);
    close(fd2);

    free(buf);

    return 1;
}

int sort_lib(char* name, int size, int bytes){
    FILE* fp = fopen(name, "r+");

    if (fp == NULL) {
        perror(name);
        return 0;
    }

    char* buf1 = calloc(bytes, sizeof(char*));
    char* buf2 = calloc(bytes, sizeof(char*));

    for(int i = 0; i < size; ++i){
        int min = i;

        if(fseek_then_freed(fp, min, 1, buf1) == 0){
            return 0;
        }

        for(int j = i + 1; j < size; ++j){

            if(fseek_then_freed(fp, j, 1, buf2) == 0){
                return 0;
            }

            if(buf1[0] > buf2[0]){
                min = j;

                if(fseek_then_freed(fp, min, 1, buf1) == 0){
                    return 0;
                }
            }
        }

        if(min != i){
            if(fseek_then_freed(fp, i, bytes, buf1) == 0){
                return 0;
            }
            if(fseek_then_freed(fp, min, bytes, buf2) == 0){
                return 0;
            }

            if(fseek_then_fwrite(fp, i, bytes, buf2) == 0){
                return 0;
            }
            if(fseek_then_fwrite(fp, min, bytes, buf1) == 0){
                return 0;
            }
        }
    }   

    fclose(fp);

    free(buf1);
    free(buf2);

    return 1;
}

int copy_lib(char* name1, char* name2, int size, int bytes){
    FILE* fp1 = fopen(name1, "r");

    if (fp1 == NULL) {
        perror(name1);
        return 0;
    }

    FILE* fp2 = fopen(name2, "w");

    if (fp2 == NULL) {
        perror(name2);
        return 0;
    }

    char* buf = calloc(bytes, sizeof(char*));

    for(int i = 0; i < size; ++i){
        fread(buf, bytes, 1, fp1);
        fwrite(buf, bytes, 1, fp2);
    }

    fclose(fp1);
    fclose(fp2);

    free(buf);
    return 1;
}

int main(int argc, char** argv){
    
    char* buffer = NULL; // BUFFER DO TWORZENIA LOGU

    for(int i = 1; i < argc; ++i){
        if(strcmp(argv[i], "generate") == 0){
            if(argc - i - 1 < 3){
                printf("Za malo argumentow.\n");
                break;
            }

            char* name = argv[i + 1];
            int size = atoi(argv[i + 2]);
            int bytes = atoi(argv[i + 3]);

            if(size < 0 || bytes < 0){
                printf("Liczba nie moe byc ujemna: %d %d\n", size, bytes);
                continue;
            }

            generate(name, size, bytes);

            i += 3;
        }else if(strcmp(argv[i], "sort") == 0){
            if(argc - i - 1 < 4){
                printf("Za malo argumentow.\n");
                break;
            }

            char* name = argv[i + 1];
            int size = atoi(argv[i + 2]);
            int bytes = atoi(argv[i + 3]);
            char* mode = argv[i + 4];

            if(size < 0 || bytes < 0){
                printf("Liczba nie moze byc ujemna: %d %d\n", size, bytes);
                i += 4;
                continue;
            }

            if(strcmp(mode, "sys") == 0){
                if(sort_sys(name, size, bytes) == 0){
                    printf("Blad podczas sortowania pliku %s w trybie %s. \n", name, mode);
                    return -1;
                }
            }else if(strcmp(mode, "lib") == 0){
                if(sort_lib(name, size, bytes) == 0){
                    printf("Blad podczas sortowania pliku %s w trybie %s. \n", name, mode);
                    return -1;
                }
            }else{
                printf("Nieznany tryb sortowania: %s \n", mode);
            }

            i += 4;
        }else if(strcmp(argv[i], "copy") == 0){
            if(argc - i - 1 < 5){
                printf("Za malo argumentow.\n");
                break;
            }

            char* name1 = argv[i + 1];
            char* name2 = argv[i + 2];
            int size = atoi(argv[i + 3]);
            int bytes = atoi(argv[i + 4]);
            char* mode = argv[i + 5];

            if(size < 0 || bytes < 0){
                printf("Liczba nie moze byc ujemna: %d %d\n", size, bytes);
                continue;
            }

            if(strcmp(mode, "sys") == 0){
                if(copy_sys(name1, name2, size, bytes) == 0){
                    printf("Blad podczas kopiowania plikow %s -> %s w trybie %s. \n", name1, name2, mode);
                    return -1;
                }
            }else if(strcmp(mode, "lib") == 0){
                if(copy_lib(name1, name2, size, bytes) == 0){
                    printf("Blad podczas kopiowania plikow %s -> %s w trybie %s. \n", name1, name2, mode);
                    return -1;
                }
            }else{
                printf("Nieznany tryb kopiowania: %s \n", mode);
            }
            
            i += 5;
        }else if(strcmp(argv[i], "log") == 0){
            if(argc - i - 1 < 1){
                printf("Za malo argumentow.\n");
                break;
            }
            log_buffer(buffer, argv[i + 1]);
            i += 1;
        }else if(strcmp(argv[i], "start_clock") == 0){
            start_clock();
        }else if(strcmp(argv[i], "stop_clock") == 0){
            stop_clock(&buffer);
        }
    }

    return 0;
}
