#ifndef _FIND_LIB_H_
#define _FIND_LIB_H_

void create_table(int size);

void free_table();

void set_targets(char dir_path[], char file_name[]);

void search_directory(char tmp_file_name[]);

int load_to_memory();

void remove_block(int index);

int is_table_initialized();

#endif