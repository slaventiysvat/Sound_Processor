#ifndef S_MALLOC
#define S_MALLOC
// n - rows
// m - columm
float* memory_allocate1D(int n, int m);

float** memory_allocate2D(int n, int m);

void memory_deallocate1D(float* arr);

//void memory_deallocate2D(float** arr, int m, int n);

#endif/*S_MALLOC*/