#ifndef SPECIAL_MATH
#define SPECIAL_MATH


typedef struct matrix_array {
	int columns;
	int rows;
	float** matrix_arr;
}matrix_array;

typedef struct vector_array {
	int length;
	float*vector_arr;
}vector_array;

/* ------------------------------------------------------*/
//Outputting a two-dimensional matrix to the console
//columns_0 _____________      
//  rows_0 |             |    
//  rows_1 | matrix(i,j) |  
//  rows_2 |_____________|     
//--------------------------------------------------------/
void print_matrix_(float** matrix, int rows,int columns);

/* ------------------------------------------------------*/
//Output of a one-dimensional matrix to the console    
//--------------------------------------------------------/
void print_vector(float* matrix, int length);

matrix_array matrix_create(int rows, int columns);

void matrix_delete(matrix_array matrix_array);

matrix_array matrix_struct_fill(float** matr_array, int rows, int columns);

//controll dimension of 2D arrays and returning correct
//dimension for result of matrix multiply
int ret_dim_of_new_matrix(int m_0, int n_0, int m_1, int n_1, int* m_3, int* n_3);

int matrix_mpy(matrix_array* matrix_1, matrix_array* matrix_2, matrix_array* matrix_3);

int matrix_summ(matrix_array* matrix_1, matrix_array* matrix_2, matrix_array* matrix_3);

int matrix_sub(matrix_array* matrix_1, matrix_array* matrix_2, matrix_array* matrix_3);

int matrix_mul_cosnt(matrix_array* matrix_1, matrix_array* matrix_2, float const value);

int matrix_div_cosnt(matrix_array* matrix_1, matrix_array* matrix_2, float const value);

#endif/*SPECIAL_MATH*/
