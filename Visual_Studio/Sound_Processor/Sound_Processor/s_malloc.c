#include "s_malloc.h"
#include "stdlib.h"
float* memory_allocate1D(int n, int m) {
 
    float *arr = (float*)malloc(n * m * sizeof(float));

    return arr;
}

float** memory_allocate2D(int n, int m) {

    int i = 0;
    int sizeof_rows = n * sizeof(float*);
    int sizeof_column = m * sizeof(float);
    float** arr = (float**)malloc(sizeof_rows);
    for (i; i < n; i++) {
        arr[i] = (float*)malloc(sizeof_column);
    }
    return arr;

}