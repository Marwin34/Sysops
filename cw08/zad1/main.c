#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>

#include "image.h"
#include "filter.h"

#define BLOCK 0
#define INTERVALED 1

static int threads_count;
static int threads_type;
static char *image_path;
static char *filter_path;
static char *out_image_path;

img_t image;
img_t out_image;
filter_t filter;

void timeval_diff(struct timeval* start, struct timeval* end, struct timeval* diff){
    if(end->tv_usec < start->tv_usec){
        diff->tv_sec = end->tv_sec - 1 - start->tv_sec;
        diff->tv_usec = 1000000  + end->tv_sec - start->tv_sec;
    }else{
        diff->tv_sec = end->tv_sec - start->tv_sec;
        diff->tv_usec = end->tv_usec - start->tv_usec;
    }
}

void* thread_handler(void* ptr){
    struct timeval start;
    gettimeofday(&start, NULL);

    int* thread_num = (int*)ptr;
    if(threads_type == BLOCK){
        int left_edge_x = *thread_num * (image.width / (float)threads_count);
        int right_edge_x = (*thread_num + 1) * (image.width / (float)threads_count);

        for(int x = left_edge_x; x < right_edge_x; ++x){
            for(int y = 0; y < image.height; ++y){
                filter_apply(&filter, &image, &out_image, x, y, EDGE_EXTEND);
            }
        }
    }else if(threads_type == INTERVALED){
        int x = *thread_num;

        while(x < image.width){
            for(int y = 0; y < image.height; ++y){
                filter_apply(&filter, &image, &out_image, x, y, EDGE_EXTEND);
            }
            x += threads_count;
        }
    }

    struct timeval end;
    gettimeofday(&end, NULL);

    struct timeval* diff = malloc(sizeof(struct timeval));
    timeval_diff(&start, &end, diff);

    pthread_exit(diff);
}

int main(int argc, char** argv){
    if(argc != 6){
        printf("Too few arguments. ./main threads_count thread_type path_to_image path_to_filter out_image_path\n");
        exit(1);
    }

    if(sscanf(argv[1], "%d", &threads_count) != 1 ||
       sscanf(argv[2], "%d", &threads_type) != 1){
           printf("Invalid arguments! \n");
           exit(2);
    }

    if(threads_type != BLOCK && threads_type != INTERVALED){
        printf("Invalid thread type. \n");
        exit(3);
    }

    image_path = argv[3];
    filter_path = argv[4];
    out_image_path = argv[5];

    img_load(image_path, &image);
    img_new(&out_image, &image);

    filter_load(filter_path, &filter);

    struct timeval start;
    gettimeofday(&start, NULL);

    pthread_t* threads = malloc(sizeof(pthread_t) * threads_count);
    int *indexes = malloc(sizeof(int) * threads_count);

    for(int i = 0; i < threads_count; ++i){
        indexes[i] = i;
        pthread_create(&threads[i], NULL, thread_handler, &indexes[i]);
    }

    struct timeval *thread_result;
    for(int i = 0; i < threads_count; ++i){
        pthread_join(threads[i], (void **)&thread_result);
        printf("Thread[%d] with id: %ld finished task in %lds %ldus. \n", i, threads[i], thread_result->tv_sec, thread_result->tv_usec);
        free(thread_result);
    }

    struct timeval end;
    gettimeofday(&end, NULL);

    struct timeval diff;
    timeval_diff(&start, &end, &diff);

    printf("Finished in: %2lds %6ldus.\n", diff.tv_sec, diff.tv_usec);

    img_save(out_image_path, &out_image);

    free(threads);
    free(indexes);
    filter_deallocate(&filter);
    img_deallocate(&image);
    img_deallocate(&out_image);

    return 0;
}