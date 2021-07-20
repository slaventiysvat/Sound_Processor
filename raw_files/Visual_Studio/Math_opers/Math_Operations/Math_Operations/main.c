#include "special_math.h"
#include "s_malloc.h"

float A[6][6] = { { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 }, // matrix 6x6
              { 7.0, 8.0, 9.0, 10.0, 11.0, 12.0 },
              { 13.0, 14.0, 15.0, 16.0, 17.0, 18.0 },
              { 19.0, 20.0, 21.0, 22.0, 23.0, 24.0 },
              { 25.0, 26.0, 27.0, 28.0, 29.0, 30.0 },
              { 31.0, 32.0, 33.0, 34.0, 35.0, 36.0 } };

float B[6][4] = { {1.0, 2.0, 3.0, 5.0},
        {7.0, 8.0, 10.0, 25.0},
        {19.0, 20.0, 21.0, 48.0},
        {19.0, 20.0, 21.0, 66.0},
        {19.0, 20.0, 21.0, 24.0},
        {19.0, 20.0, 21.0, 30.0 }};   //matrix 6x4


matrix_array matrix_A;
matrix_array matrix_B;
matrix_array matrix_C;

int main(void) {

    matrix_A.columns = 6;
    matrix_A.rows = 6;
    matrix_A.matrix_arr = A;

    matrix_B.columns = 4;
    matrix_B.rows = 6;
    matrix_B.matrix_arr = B;

    int new_rows;
    int new_columns;

    int answer = ret_dim_of_new_matrix(matrix_A.rows,
        matrix_A.columns,
        matrix_B.rows,
        matrix_B.columns,
        &new_rows,
        &new_columns);

    if (answer == -1) {
    
        return -1;
    
    }

    float** C = memory_allocate2D(new_rows, new_columns);

    


    //Необходимо удалить массив С и создать в управлетиле памятью удаление этого массива
	return 0;

}