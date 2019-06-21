#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "filter.h"


int main(){
    char buffer[255];

    srand(time(0));

    for(int i = 3; i <= 65; i += 2){
        filter_t filter;
        filter_allocate(&filter, i);

        int index = 0;

        for(int y = 0; y < i; ++y){
            for(int x = 0; x < i; ++x){
                filter.array[index++] = (float)(rand() % 1000) / 1000;
            }
        }

        filter_normalize(&filter);
        snprintf(buffer, 255, "generated_filters/filter_%d.txt", i);
        filter_save(buffer, &filter);
        filter_deallocate(&filter);
    }

    return 0;
}