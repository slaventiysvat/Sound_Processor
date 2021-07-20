#include "s_malloc.h"
#include "stdlib.h"
/*Allocating memory for one dimension array*/
float* memory_allocate1D(int m, int n) {
 
    float *arr = (float*)malloc(n * m * sizeof(float));

    return arr;
}
/*Allocating memory for the two dimension array*/
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
/*Deallocating memory for the one dimension array*/
void memory_deallocate1D(float* arr) {

    free(arr);

}
/*Deallocating memory for two dimension array*/
void memory_deallocate2D(float** arr[ROWS][COLUMNS], int m, int n) {

    int i = 0;

    for (i; i < m; i++) {
        free(arr[i]);
    }

    free(arr);
}