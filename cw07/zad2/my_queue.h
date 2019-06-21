#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "shared.h"

#define MAX_PACKAGES_COUNT 32


struct my_queue{
    int id;
    int M;
    int K;
    int packages_count;
    int packages_mass;

    char* path;
    struct package packages[MAX_PACKAGES_COUNT];
};

struct my_queue* create_queue(char* path);

struct my_queue* open_queue(char* path);

void close_queue(struct my_queue* ptr);

void init_queue(struct my_queue* ptr, int M, int K);

struct package top_value(struct my_queue* ptr, int max_payload, int current_payload);

int put_on_queue(struct my_queue* ptr, int val, pid_t PID, struct timeval ttime);

sem_t* create_semaphore(char* sem_name);

sem_t* open_semaphore(char* sem_name);

void lock_semaphore(sem_t* id);

void unlock_semaphore(sem_t* id);

void close_semaphore(sem_t* id);

void unlink_semaphore(char* name);