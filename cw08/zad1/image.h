#pragma once

#define GRAY 1

typedef struct img_t
{
    int *array;
    int width;
    int height;
    int max_value;
    int color_mode;
} img_t;

int img_allocate(img_t *img, int width, int height, int max_value, int color_mode);
int img_deallocate(img_t *img);

int img_load(char *path, img_t *result);
int img_save(char *path, img_t *img);

int img_new(img_t *result, img_t *img);
int get_pixel_index(img_t *img, int x, int y, int *index);