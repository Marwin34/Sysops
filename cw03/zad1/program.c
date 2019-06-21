#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <time.h>
#include <wait.h>

#define PATH_MAX 4096


void traverse_dricetory_tree(char* path, char relative_path[PATH_MAX]){
    DIR* direcotry = opendir(path);

    if(chdir(path) == -1){
        printf("Nie mozna bylo odwiedzic folderu %s. \n", path);
        return;
    }

    char * rel_path = calloc(PATH_MAX, sizeof(char*));

    strcpy(rel_path, relative_path);
    strcat(rel_path, "/");
    strcat(rel_path, path);

    pid_t child_pid;

    child_pid = fork();
    if(child_pid == -1){
        printf("Nie mozna utworzyc procesu.");
    }else if(child_pid > 0){
        int status;
        waitpid(child_pid, &status, 0);
    }else{

        char cwd[PATH_MAX];
        if(getcwd(cwd, PATH_MAX) == NULL){
            printf("Nie mozna odczytac bierzacego katalogu");
        }

        printf("%s \n", rel_path);
        execlp("ls", "ls", "-l", NULL);
    }

    if(direcotry == NULL){
        printf("Nie można otworzyć folder %s\n", path);
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

        if((dir_stat.st_mode & S_IFMT) == S_IFDIR){ 

            traverse_dricetory_tree(name, rel_path);
        }
    }  
    chdir("..");
    return;
}

int main(int argc, char** argv){
    if(argc == 2){
        traverse_dricetory_tree(argv[1], "");
    } 

    return 0;
}