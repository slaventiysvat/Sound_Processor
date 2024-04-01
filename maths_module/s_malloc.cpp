#include "pch.h"

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

/*Deallocating memory for the one dimension array*/
void memory_deallocate1D(float* arr) {

    free(arr);

}

///*Deallocating memory for two dimension array*/
//void memory_deallocate2D(float** arr, int m, int n) {
//
//    int i = 0;
//
//    for (i; i < m; i++) {
//        free(arr[i]);
//    }
//
//    free(arr);
//}
