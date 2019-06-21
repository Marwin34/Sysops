#pragma once

#include "image.h"

#define EDGE_EXTEND 1

typedef struct filter_t
{
    double *array;
    int size;
    int normalized;
} filter_t;

int filter_allocate(filter_t *filter, int size);
int filter_deallocate(filter_t *filter);

int filter_load(char *path, filter_t *out);
int filter_save(char *path, filter_t *filter);

int filter_normalize(filter_t *filter);
int filter_apply(filter_t *filter, img_t *img_src, img_t *img_out, int x, int y, int edge_mode);