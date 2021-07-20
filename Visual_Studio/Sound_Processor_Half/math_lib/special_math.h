#ifndef SPECIAL_MATH
#define SPECIAL_MATH

typedef struct matrix_array {

    int rows;
	int columns;
	float *s_vector;

}matrix_array;

matrix_array matrix_struct_fill(float* matr_array, int rows, int columns);

struct matrix_array* matrix_create(int rows, int columns);

void matrix_delete(matrix_array* unuseful_array);

//struct matrix_array* s_transponze(struct matrix_array* mat);

/*returns an n - by - m matrix with "val" on the main diagonal and zeros elsewhere.*/
void s_eye(matrix_array* mat, float val);

/*returns an n - by - m matrix with "zero values" on the matrix.*/
void s_zeros(matrix_array* mat);

/*returns an n - by - m matrix with "one values" on the matrix.*/
void s_ones(matrix_array* mat);
/*returns an n - by - m matrix with "values" on the matrix.*/
void s_values(matrix_array* mat,float val);

void print_matrix(matrix_array* mat);

int vector_mpy_const(matrix_array* vector_in, matrix_array* vector_out, float constant);

int vector_div_const(matrix_array* vector_in, matrix_array* vector_out, float constant);

int sum_mat(matrix_array* mat1,  matrix_array* mat2,  matrix_array* mat3);

int sub_mat(matrix_array* mat1, matrix_array* mat2, matrix_array* mat3);

int mul_mat(matrix_array* mat1, matrix_array* mat2, matrix_array* mat3);

float row_mul_column(matrix_array* mat1, matrix_array* mat2);

void copy_mat(matrix_array* mat1, matrix_array* mat2);

int ret_dim_of_new_matrix(matrix_array* mat1, matrix_array* mat2, matrix_array* mat3);

void matrix_transponze(matrix_array* matrix_1, matrix_array* matrix_2);

#endif/*SPECIAL_MATH*/
