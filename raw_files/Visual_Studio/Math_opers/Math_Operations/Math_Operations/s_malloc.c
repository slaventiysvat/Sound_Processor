#include "s_malloc.h"
#include "stdlib.h"
float* memory_allocate1D(int m, int n) {
 
    float *arr = (float*)malloc(n * m * sizeof(float));

    return arr;
}

float** memory_allocate2D(int m, int n) {

    int i = 0;
    int sizeof_rows = m * sizeof(float*);
    int sizeof_column = n * sizeof(float);
    float** arr = (float**)malloc(sizeof_rows);
    for (i; i < n; i++) {
        arr[i] = (float*)malloc(sizeof_column);
    }
    return arr;

}