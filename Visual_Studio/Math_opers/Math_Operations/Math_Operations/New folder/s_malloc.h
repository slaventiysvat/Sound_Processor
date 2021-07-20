#ifndef S_MALLOC
#define S_MALLOC
// m - rows
// n - columm
#define ROWS    500
#define COLUMNS 500
float* memory_allocate1D(int m, int n);

void memory_deallocate1D(float* arr);

float** memory_allocate2D(int m, int n);

void memory_deallocate2D(float** arr[ROWS][COLUMNS], int m, int n);

#endif/*S_MALLOC*/