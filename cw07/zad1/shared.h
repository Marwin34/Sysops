#include <sys/types.h>
#include <sys/time.h>


struct package{
    int m;
    pid_t pid;   
    struct timeval time;
};