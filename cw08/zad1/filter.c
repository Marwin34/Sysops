#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "filter.h"


#define eps 0.0000001

int filter_allocate(filter_t* filter, int size){
    filter->size = size;
    filter->array = malloc(sizeof(double) * size * size);
    filter->normalized = 0;

    return 0;
}

int filter_deallocate(filter_t* filter){
    filter->size = 0;
    free(filter->array);
    filter->normalized = 0;
    return 0;
}

static int transform_coords(int* x, int* y, img_t* img, int edge_mode){
    switch (edge_mode)
    {
    case EDGE_EXTEND:
        if(*x < 0){
            *x = 0;
        }
        if(*y < 0){
            *y = 0;
        }
        if(*x >= img->width){
            *x = img->width - 1;
        }
        if(*y >= img->height){
            *y = img->height - 1;
        }
        break;
    
    default:
        printf("Unknown edge mode. \n");
        return -1;
        break;
    }

    return 0;
}

int filter_load(char* path, filter_t* filter){
    FILE *fd;

    if((fd = fopen(path, "r")) == NULL){
        perror("Unable to open file (filter).");
        exit(7);
    }

    char buffer[1024];
    int line = 0;
    int index = 0;

    while(fgets(buffer, 1024, fd) != NULL){
        if(buffer[0] == '#'){
            continue;
        }

        if(line == 0){
            int size = 0;

            if(sscanf(buffer, "%d", &size) != 1){
                printf("Unable to parse size. \n");
                return -1;
            }
            filter_allocate(filter, size);
        }else{
            char* ptr = strtok(buffer, " \r\n");
            while(ptr != NULL){
                double tmp;
                if(sscanf(ptr, "%lf", &tmp) != 1){
                    printf("Unable to parse value. \n");
                    return -1;
                }

                filter->array[index++] = tmp;
                ptr = strtok(NULL, " \r\n");
            }
        }
        ++line;
    }

    if(index != (filter->size * filter->size)){
        printf("Unable to parse file, size missmatch. \n");
        return -1;
    }

    int sum = 0;
    int num = filter->size * filter->size;

    for(int i = 0; i < num; ++i){
        sum += filter->array[i];
    }

    if(abs(sum - 0.0) < eps || abs(sum - 1.0) < eps){
        filter->normalized = 1;
    }else{
        filter->normalized = 0;
    }

    fclose(fd);
    return 0;
}

int filter_save(char* path, filter_t* filter){
    FILE* fd;

    if((fd = fopen(path, "w")) == NULL){
        perror("Unable to open file (filter).");
        exit(8);
    }

    fprintf(fd, "%d", filter->size);

    for(int i = 0; i < filter->size * filter->size; ++i){
        if(i % filter->size == 0){
            fprintf(fd, "\n");
        }
        fprintf(fd, "%.10f ", filter->array[i]);
    }

    fclose(fd);
    return 0;
}

int filter_normalize(filter_t* filter){
    double sum = 0;
    int num = filter->size * filter->size;

    for(int i = 0; i < num; ++i){
        sum += filter->array[i];
    }

    if(sum == 0){
        filter->normalized = 1;
        return 0;
    }

    for(int i = 0; i < num; ++i){
        filter->array[i] /= sum;
    }

    filter->normalized = 1;
    
    return 0;
}

int filter_apply(filter_t* filter, img_t* img_src, img_t* img_out, int x, int y, int edge_mode){
    if(filter->normalized == 0){
        printf("Filter nor normalized \n");
        return -1;
    }

    double pixel;
    pixel = 0;

    int left_offset = -filter->size / 2;
    int right_offset = filter->size / 2;

    if(filter->size % 2 == 0){
        right_offset--;
    }

    int filter_index = 0;
    for(int p_y = y + left_offset; p_y <= y + right_offset; ++p_y){
        for(int p_x = x + left_offset; p_x <= x + right_offset; ++p_x){
            int p_x_2 = p_x;
            int p_y_2 = p_y;

            if(transform_coords(&p_x_2, &p_y_2, img_src, edge_mode) < 0){
                return -1;
            }

            int pixel_index;
            get_pixel_index(img_src, p_x_2, p_y_2, &pixel_index);

            switch (img_src->color_mode)
            {
            case GRAY:
                pixel += filter->array[filter_index] * img_src->array[pixel_index];
                break;

            default:
                printf("Unknown color mode (filter). \n");
                return -1;
                break;
            }

            ++filter_index;
        } 

        if(pixel < 0){
            pixel = 0;
        }
        if(pixel > img_out->max_value){
            pixel = img_out->max_value;
        }

        int out_pixel_index;
        get_pixel_index(img_out, x, y, &out_pixel_index);

        switch (img_src->color_mode)
        {
        case GRAY:
            img_out->array[out_pixel_index] = (int)pixel;
            break;

        default:
            printf("Unknown color mode (filter 2). \n");
            return -1;
            break;
        }
    }

    return 0;
}