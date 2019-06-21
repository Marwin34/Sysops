#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <time.h>


time_t make_time_from_string(char* date){
    char year_s[5];
    char month_s[3];
    char day_s[3];
    char hour_s[3];
    char minutes_s[3];
    char seconds_s[3];

    strncpy(year_s, date + 0, 4);
    strncpy(month_s, date + 5, 2);
    strncpy(day_s, date + 8, 2);
    strncpy(hour_s, date + 11, 2);
    strncpy(minutes_s, date + 14, 2);
    strncpy(seconds_s, date + 17, 2);

    year_s[4] = '\0';
    month_s[2] = '\0';
    day_s[2] = '\0';
    hour_s[2] = '\0';
    minutes_s[2] = '\0';
    seconds_s[2] = '\0';

    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));

    tm.tm_year = atoi(year_s) - 1900;
    tm.tm_mon = atoi(month_s) - 1;
    tm.tm_mday = atoi(day_s);
    tm.tm_hour = atoi(hour_s);
    tm.tm_min = atoi(minutes_s);
    tm.tm_sec = atoi(seconds_s);
    tm.tm_isdst = -1;

    time_t result = mktime(&tm);

    return result;
}

void print_info(struct stat sb, char* name){

    char actualpath [PATH_MAX+1];

    realpath(name, actualpath);
    printf("Sciezka absolutna:          %s \n", actualpath);

    printf("Rodzaj pliku:               ");
                                      
    switch (sb.st_mode & S_IFMT) {
        case S_IFBLK:  printf("urzadzenie blokowe\n");            break;
        case S_IFCHR:  printf("urzadzenie znakowe\n");        break;
        case S_IFDIR:  printf("katalog\n");               break;
        case S_IFIFO:  printf("potok nazwany\n");               break;
        case S_IFLNK:  printf("link symboliczny\n");                 break;
        case S_IFREG:  printf("zwykly plik\n");            break;
        case S_IFSOCK: printf("socket\n");                  break;
        default:       printf("nieznany?\n");                break;
    }
    printf("Rozmiar pliku:              %lld bajtow\n",
            (long long) sb.st_size);  
    printf("Data ostatniego dostepu:    %s", ctime(&sb.st_atime));
    printf("Data ostatniej modyfikacji: %s", ctime(&sb.st_mtime));
}

int compare_dates(time_t date1, time_t date2, char operation){
    if(operation == '>'){
        if(difftime(date1, date2) > 0){
            return 1;
        }else{
            return 0;
        }
    }else if(operation == '<'){
        if(difftime(date1, date2) < 0){
            return 1;
        }else{
            return 0;
        }
    }else if(operation == '='){
        if(difftime(date1, date2) == 0){
            return 1;
        }else{
            return 0;
        }
    }
    return 0;
}

void traverse_dricetory_tree(char* path, char operation, time_t date){
    if(chdir(path) == -1){
        printf("Nie mozna otworzyc folderu: %s", path);
        return;
    }

    DIR* direcotry = opendir(".");

    if(direcotry == NULL){
        printf("Nie można otworzyć folderu %s\n", path);
        chdir("..");
        return;
    }
    
    struct dirent* dit;

    while((dit = readdir(direcotry)) != NULL){
        char* name = dit->d_name;

        if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0){
            continue;
        }

        struct stat dir_stat;
        if(lstat(name, &dir_stat) == -1){
            printf("Nie można odczytać statystkyk: %s\n", name);
            continue;
        }  

        if(compare_dates(dir_stat.st_mtime, date, operation) == 1){
            print_info(dir_stat, name);
        }

        if((dir_stat.st_mode & S_IFMT) == S_IFDIR){ 
            traverse_dricetory_tree(name, operation, date);
        }
    }  
    chdir("..");
    return;
}


int main(int argc, char** argv){
    if(argc == 4){
        //DATE FORMAT YYYY-MM-DD-hh:mm:ss - 24h format
        if(strlen(argv[3]) == 19){
            
            time_t target = make_time_from_string(argv[3]);
            traverse_dricetory_tree(argv[1], argv[2][0], target);
        }
    } 

    return 0;
}