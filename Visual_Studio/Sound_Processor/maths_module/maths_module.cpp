// maths_module.cpp : Defines the functions for the static library.
//

//#include "pch.h"
//#include "framework.h"
#include "special_math.h"
#include "s_malloc.h"
#include "stdio.h"
// TODO: This is an example of a library function
void fnmathsmodule()
{
    matrix_array Arr_1 = matrix_create(4, 6);
    printf("matrix 4x6 was created \n");
    print_matrix(&Arr_1);

    matrix_array Arr_2 = matrix_create(6, 5);
    printf("matrix 6x5 was created \n");
    print_matrix(&Arr_2);

    printf("matrix 4x6 was filled \n");
    s_values(&Arr_1, 10.0f);
    print_matrix(&Arr_1);

    printf("matrix 6x5 was filled \n");
    s_values(&Arr_2, 5.0f);
    print_matrix(&Arr_2);

    printf("matrix 4x6 was multiplyed on 5.0 \n");
    vector_mpy_const(&Arr_1, 5.0f);
    print_matrix(&Arr_1);

    printf("matrix 4x6 was divided on 5.0 \n");
    vector_div_const(&Arr_1, 5.0f);
    print_matrix(&Arr_1);

    printf("matrix 4x6 was added to itself \n");
    sum_mat(&Arr_1, &Arr_1, &Arr_1);
    print_matrix(&Arr_1);

    printf("matrix 4x6 was substracted from itself \n");
    sub_mat(&Arr_1, &Arr_1, &Arr_1);
    print_matrix(&Arr_1);

    matrix_array Arr_3;
    int answer = ret_dim_of_new_matrix(&Arr_1, &Arr_2, &Arr_3);
    Arr_3.s_vector = memory_allocate1D(Arr_3.rows, Arr_3.columns);
    s_values(&Arr_1, 10.0f);

    printf("matrix 4x6 \n");
    print_matrix(&Arr_1);

    printf("matrix 6x5 \n");
    print_matrix(&Arr_2);

    printf("matrix 4x6 was myltiplyed by matrix 6x5 the result is matrix %d x %d \n", Arr_3.rows, Arr_3.columns);
    answer = mul_mat(&Arr_1, &Arr_2, &Arr_3);
    print_matrix(&Arr_3);

    answer = mul_mat(&Arr_2, &Arr_1, &Arr_3);

    if (answer == -1) {

        printf("Matrix A can't multiply by matrix B \n");

    }

    matrix_array Arr_5 = matrix_create(3, 6);
    matrix_array Arr_6 = matrix_create(6, 3);

    s_values(&Arr_5, 26.0f);
    printf("Matrix Arr_5 before transponze \n");
    print_matrix(&Arr_5);

    matrix_transponze(&Arr_5, &Arr_6);
    printf("Matrix transponze_mat \n");


}
