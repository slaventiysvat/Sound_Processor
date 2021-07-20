#include "special_math.h"
#include <stdio.h>
#include "special_math.h"

inline void print_matrix_(float** matrix, int rows,int columns) {

	for (int i = 0; i < rows; i++) {
	
		for (int j = 0; j < columns; j++) {
		
			printf("%f, ", matrix[i][j]);
		
		}
	
		printf("\n");
	}

}

inline void print_vector(float* matrix, int length) {

	for (int i = 0; i < length; i++) {
	
		printf("%f, \n", matrix[i]);

	}

}


matrix_array matrix_struct_fill(float** matr_array, int rows, int columns) {

	matrix_array new_matrix;
	new_matrix.rows = rows;
	new_matrix.columns = columns;
	new_matrix.matrix_arr = matr_array;
	return new_matrix;

}

matrix_array matrix_create(int rows, int columns) {

	matrix_array new_matrix;

	if ((rows <= 0) || (columns <= 0)) {

		new_matrix.columns = 0;
		new_matrix.rows = 0;
		new_matrix.matrix_arr = 0;

	}

	new_matrix.columns = columns;
	new_matrix.rows = rows;
	new_matrix.matrix_arr = memory_allocate2D(new_matrix.rows, new_matrix.columns);

	return new_matrix;
}



void matrix_delete(matrix_array matrix_array) {

	//deleting an array in structure
	free((void*)(matrix_array.matrix_arr));
	//deleting of the structure itself
	free((void*)(&matrix_array));

}


inline int ret_dim_of_new_matrix(int m_0, int n_0, int m_1, int n_1, int* m_3, int* n_3) {

	if (n_0 =! m_1) {
		*m_3 = 0;
		*n_3 = 0;
		printf("The input dimension is incorrect");
		return -1;
	}

	*m_3 = n_0;//rows
	*n_3 = n_1;//columns

	return 0;
}

int matrix_mpy(matrix_array* matrix_1, matrix_array* matrix_2, matrix_array* matrix_3) {

	int new_columns = 0;
	int new_rows = 0;
	int answer = ret_dim_of_new_matrix(matrix_1->columns,
		matrix_1->rows, 
		matrix_2->columns, 
		matrix_2->rows, 
		&new_rows,
		&new_columns);

	if (answer == -1) {
	
		return answer;
	
	}

	matrix_3->columns = new_columns;
	matrix_3->rows = new_rows;

	for (int i = 0; i < matrix_1->rows; i++) {

		for (int j = 0; j < matrix_1->columns; j++) {

			matrix_3->matrix_arr[i][j] = 0;

			for (int k = 0; k < matrix_2->columns; k++) {

				matrix_3->matrix_arr[i][j] = matrix_1->matrix_arr[i][k] * matrix_2->matrix_arr[k][j];

			}
		}

	}

	return answer;

}

int matrix_summ(matrix_array* matrix_1, matrix_array* matrix_2, matrix_array* matrix_3) {

	int answer = 0;

	if (matrix_1->columns != matrix_2->columns) {

		answer = -1;
		matrix_3->columns = 0;
		matrix_3->rows = 0;
		return answer;
	}
	else if (matrix_1->rows!= matrix_2->rows) {
	
		answer = -1;
		matrix_3->columns = 0;
		matrix_3->rows = 0;
		return answer;
	}
	else {
	
		matrix_3->columns = matrix_1->columns;
		matrix_3->rows = matrix_1->rows;
	
	}

	for (int i = 0; i < matrix_1->rows; i++) {

		for (int j = 0; j < matrix_1->columns; j++) {

			matrix_3->matrix_arr[i][j] = matrix_1->matrix_arr[i][j] + matrix_2->matrix_arr[i][j];


		}

	}

	return answer;
}

int matrix_sub(matrix_array* matrix_1, matrix_array* matrix_2, matrix_array* matrix_3) {

	int answer = 0;

	if (matrix_1->columns != matrix_2->columns) {

		answer = -1;
		matrix_3->columns = 0;
		matrix_3->rows = 0;
		return answer;
	}
	else if (matrix_1->rows != matrix_2->rows) {

		answer = -1;
		matrix_3->columns = 0;
		matrix_3->rows = 0;
		return answer;
	}
	else {

		matrix_3->columns = matrix_1->columns;
		matrix_3->rows = matrix_1->rows;

	}

	for (int i = 0; i < matrix_1->rows; i++) {

		for (int j = 0; j < matrix_1->columns; j++) {

			matrix_3->matrix_arr[i][j] = matrix_1->matrix_arr[i][j] - matrix_2->matrix_arr[i][j];


		}

	}

	return answer;

}

int matrix_mul_cosnt(matrix_array* matrix_1, matrix_array* matrix_2, float const value) {

	int answer = 0;

	if ((matrix_1->columns <= 0) || (matrix_1->rows <= 0)) {
	
		matrix_2->columns = 0;
		matrix_2->rows = 0;
		answer = -1;
		return answer;
	}
	else {
	
		matrix_2->columns = matrix_1->columns;
		matrix_2->rows = matrix_1->rows;
	
	}


	for (int i = 0; i < matrix_1->rows; i++) {

		for (int j = 0; j < matrix_1->columns; j++) {

			matrix_2->matrix_arr[i][j] = matrix_1->matrix_arr[i][j] * value;


		}

	}
	return answer;
}

int matrix_div_cosnt(matrix_array* matrix_1, matrix_array* matrix_2, float const value) {

	int answer = 0;

	if ((matrix_1->columns <= 0) || (matrix_1->rows <= 0)) {

		matrix_2->columns = 0;
		matrix_2->rows = 0;
		answer = -1;
		return answer;
	}
	else {

		matrix_2->columns = matrix_1->columns;
		matrix_2->rows = matrix_1->rows;

	}


	for (int i = 0; i < matrix_1->rows; i++) {

		for (int j = 0; j < matrix_1->columns; j++) {

			matrix_2->matrix_arr[i][j] = matrix_1->matrix_arr[i][j] / value;


		}

	}
	return answer;

}