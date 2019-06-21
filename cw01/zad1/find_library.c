#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "find_library.h"

static char** table;

static int table_size;

static char* current_dir;

static char* target_file;

static char* tmp_file_path;

static int initialized = 0;

void create_table(int size){
    table = (char**)calloc(size, sizeof(char*));

    table_size = size;

    initialized = 1;
}

void free_table(){ 
    if(initialized == 0){
        return;
    }
    
    for(int i = 0; i < table_size; ++i){
        if(table[i] != NULL){
            free(table[i]);
            table[i] = NULL;
        }
    }

    free(table);
    table = NULL;
}

void set_targets(char dir_path[], char file_name[]){
    if(current_dir != NULL){
        free(current_dir);
    }

    if(target_file != NULL){
        free(target_file);
    }

    current_dir = calloc(strlen(dir_path) + 1, sizeof(char*));
    target_file = calloc(strlen(file_name) + 1, sizeof(char*));

    strcpy(current_dir, dir_path);
    strcpy(target_file, file_name);
}

void search_directory(char tmp_file_name[]){
    tmp_file_path = calloc(strlen(tmp_file_name) + strlen("/tmp/") + 1, sizeof(char*));

    sprintf(tmp_file_path, "/tmp/%s", tmp_file_name);

    size_t command_size = strlen(current_dir) + strlen(target_file) + strlen(tmp_file_path) + strlen("find \"\" -name\"\" > \"\" 2> /dev/null" + 1);

    char* command = calloc(command_size, sizeof(char *));

    sprintf(command, "find \"%s\" -name \"%s\" > \"%s\" 2> /dev/null ", current_dir, target_file, tmp_file_path);

    system(command);

    free(command);
}

int load_to_memory(){
    FILE *fp;

    fp = fopen(tmp_file_path, "r");

    fseek(fp, 0L, SEEK_END);

    long file_size = ftell(fp);

    rewind(fp);

    int index = -1;

    for(int i = 0; i < table_size; i++){
        if(table[i] == NULL){
            index = i;
            break;
        }
    }

    if(index == -1){
        printf("Allocated array of pointer is too small.");
        return index;
    }

    table[index] = (char*)calloc(file_size, sizeof(char*));

    size_t n = 0;
    int c;

    if (fp == NULL)
        return -1; //could not open file

    while ((c = fgetc(fp)) != EOF)
    {
        table[index][n++] = (char) c;
    }

    fclose(fp);

    table[index][n] = '\0';   

    return index;
}

void remove_block(int index){
    if(index < 0 || index >= table_size){
        printf("Index %d out of table boundary.", index);
    }

    free(table[index]);
    table[index] = NULL;
}
