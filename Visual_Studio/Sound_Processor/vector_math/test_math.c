#include <stdio.h>
#include <conio.h>
#include "special_math.h"
#include "s_malloc.h"

struct s_matrix_eny mat1;
struct s_matrix_eny mat2;
struct s_matrix_eny mat3;

int main(void) {
	mat1.colum = 10; //Столбец
	mat1.rows = 5;  //Строка
	mat1.s_vector = memory_allocate1D(mat1.rows, mat1.colum);
	float val = 2.0;
	s_values(&mat1, val);

	mat2.colum = 5;
	mat2.rows = 10;
	mat2.s_vector = memory_allocate1D(mat2.rows, mat2.colum);
	s_values(&mat2, val);

	mat3.colum = mat1.rows;
	mat3.rows = mat2.colum;
	mat3.s_vector = memory_allocate1D(mat3.rows, mat3.colum);

	s_zeros(&mat3);
	mul_mat(&mat1, &mat2, &mat3);
	print_matrix(&mat1);
	printf("\n");
	print_matrix(&mat2);
	printf("\n");
	print_matrix(&mat3);

	s_eye(&mat3, val);

	printf("Transponze Matrix \n");
	mat3 = *s_transponze(&mat3);
	print_matrix(&mat3);
	printf("\n");

	mat3 = *s_transponze(&mat3);
	print_matrix(&mat3);
	printf("\n");

	return 0;
}