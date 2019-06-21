#include "image.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int get_img_array_size(img_t* img){
    if(img->color_mode == GRAY){
        return img->width * img->height;
    }else
    {
        return -1;
    }
}

int img_allocate(img_t* img, int w, int h, int max_value, int color_mode){
    img->color_mode = color_mode;
    img->width = w;
    img->height = h;
    img->max_value = max_value;
    img->array = calloc(get_img_array_size(img), sizeof(int));
    return 0;
}

int img_deallocate(img_t* img){
    img->color_mode = 0;
    img->width = 0;
    img->height = 0;
    img->max_value = 0;
    free(img->array);
    return 0;
}

int img_load(char* path, img_t* result){
    FILE* fd;

    if((fd = fopen(path, "r")) == NULL){
        perror("Unable to fopen.");
        exit(4);
    }

    char buffer[2048];
    int line = 0;
    int index = 0;

    while(fgets(buffer, 2048, fd) != NULL){
        if(buffer[0] == '#'){
            continue;
        }

        if(line == 0){
            if(strncmp("P2", buffer, 2) == 0){
                result->color_mode = GRAY;
            }else{
                printf("Unknow image format. \n");
                return -1;
            }
        }else if(line == 1){
            char* ptr = strtok(buffer, " \r\n");
            int i = 0;
            int w = 0;
            int h = 0;

            while(ptr != NULL){
                if(i == 0){
                    if(sscanf(ptr, "%d", &w) != 1){
                        printf("Unable to parse width. \n");
                        return -1;
                    }  
                }else if (i == 1){
                    if(sscanf(ptr, "%d", &h) != 1){
                        printf("Unable to parse height. \n");
                        return -1;
                    }
                }
                ++i;  
                ptr = strtok(NULL, " \r\n");
            }

            if(i != 2){
                printf("Unable to parse file, unknown width or height. \n");
                return -1;
            }

            result->width = w;
            result->height = h;
        }else if(line == 2){
            int max_val = 0;

            if(sscanf(buffer, "%d", &max_val) != 1){
                printf("Unable to parse file, cant load max_value. \n");
                return -1;
            }

            result->max_value = max_val;

            result->array = calloc(get_img_array_size(result), sizeof(int));
        }else{
            char* ptr = strtok(buffer, " \r\n");
            while(ptr != NULL){
                int tmp;
                if(sscanf(ptr, "%d", &tmp) != 1){
                    printf("Unable to parse pixel value. \n");
                    return -1;
                }

                result->array[index++] = tmp;
                ptr = strtok(NULL, " \r\n");
            }
        }
        ++line;
    }

    if(index != get_img_array_size(result)){
        printf("Size missmatch. \n");
        return -1;
    }

    fclose(fd);
    return 0;
}

int img_save(char* path, img_t* img){
    FILE* fd;

    if((fd = fopen(path, "w")) == NULL){
        perror("Unable to open file.");
        exit(5);
    }

    switch (img->color_mode)
    {
    case GRAY:
        fprintf(fd, "%s", "P2\n");
        break;

    default:
        printf("Unknow mode (img_save). \n");
        return -1;
        break;
    }

    fprintf(fd, "%d %d\n", img->width, img->height);

    fprintf(fd, "%d", img->max_value);

    for(int i = 0; i < get_img_array_size(img); ++i){
        if(i % 15 == 0){
            fprintf(fd, "%s", "\n");
        }

        fprintf(fd, "%d ", img->array[i]);
    }

    fclose(fd);
    return 0;
}

int img_new(img_t* result, img_t* img){
    return img_allocate(result, img->width, img->height, img->max_value, img->color_mode);
}

int get_pixel_index(img_t* img, int x, int y, int* index){
    *index = x + (y * img->width);

    if(*index >= get_img_array_size(img)){
        return -1;
    }
        
    return 0;
}