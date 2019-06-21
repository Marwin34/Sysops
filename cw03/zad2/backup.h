char* generate_backup_name(char* target);

int memory_backup(char* target, struct stat sb);

int execute_backup(char* target, struct stat sb);