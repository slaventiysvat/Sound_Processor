#include "special_math.h"
#include "stdio.h"

//struct matrix_array* s_transponze(struct matrix_array* mat) {
//
//    int new_colum = mat->rows;
//    int new_rows = mat->colum;
//    mat->colum = new_colum;
//    mat->rows = new_rows;
//    return mat;
//
//}

/*Passing matrix parameters to a structure*/
matrix_array matrix_struct_fill(float* matr_array, int rows, int columns) {

    matrix_array new_matrix;
    new_matrix.rows = rows;
    new_matrix.columns = columns;
    new_matrix.s_vector = matr_array;
    return new_matrix;

}

int ret_dim_of_new_matrix(matrix_array* mat1, matrix_array* mat2,matrix_array* mat3) {

    int columns = mat1->columns;
    int rows = mat2->rows;

    if (columns == rows) {

        mat3->rows = mat1->rows;//rows
        mat3->columns = mat2->columns;//columns
    }
    else {

        mat3->rows = 0;
        mat3->columns = 0;
        printf("The input dimension is incorrect \n");
        return -1;

    }

   

    return 0;

}

matrix_array matrix_create(int rows, int columns) {

   matrix_array new_matrix;

    if ((rows <= 0) || (columns <= 0)) {

        new_matrix.columns = 0;
        new_matrix.rows = 0;
        new_matrix.s_vector = 0;

    }

    new_matrix.columns = columns;
    new_matrix.rows = rows;
    new_matrix.s_vector = memory_allocate1D(new_matrix.rows, new_matrix.columns);
    return new_matrix;
}

/*returns an n - by - m matrix with "val" on the main diagonal and zeros elsewhere.*/
void s_eye(matrix_array* mat, float val) {
    int m = mat->columns;
    int n = mat->rows;
    int i = 0;
    int j = 0;
    float* pvector = mat->s_vector;
    for (i; i < n; i++) {

        float* pvector_tmp = pvector + (i * m);

        for (j; j < m; j++) {

            if (i == j) {

                *(pvector_tmp + j) = val;
            }
            else {

                *(pvector_tmp + j) = 0.0f;

            }
        }
        j = 0;
    }
}

/*returns an n - by - m matrix with "zero values" on the matrix.*/
void s_zeros(matrix_array* mat) {
    int m = mat->columns;
    int n = mat->rows;
    int i = 0;
    int j = 0;
    float* pvector = mat->s_vector;
    for (i; i < n; i++) {

        float* pvector_tmp = pvector + (i * m);

        for (j; j < m; j++) {

                *(pvector_tmp + j) = 0.0f;

        }
        j = 0;
    }
}


/*returns an n - by - m matrix with "one values" on the matrix.*/
void s_ones(matrix_array* mat) {
    int m = mat->columns;
    int n = mat->rows;
    int i = 0;
    int j = 0;
    float* pvector = mat->s_vector;
    float val = 1.0f;
    for (i; i < n; i++) {

        float* pvector_tmp = pvector + (i * m);

        for (j; j < m; j++) {

            *(pvector_tmp + j) = val;

        }
        j = 0;
    }
}

void s_values(matrix_array* mat, float val) {

    int m = mat->columns;
    int n = mat->rows;
    int i = 0;
    int j = 0;
    float* pvector = mat->s_vector;
    for (i; i < n; i++) {

        float* pvector_tmp = pvector + (i * m);

        for (j; j < m; j++) {

            *(pvector_tmp + j) = val;

        }
        j = 0;
    }
}

void print_matrix(matrix_array* mat) {
    int m = mat->columns;
    int n = mat->rows;
    int i = 0;
    int j = 0;
    float* pvector = mat->s_vector;
    float val = 1.0f;
    for (i; i < n; i++) {
        // loop on the rows
        float* pvector_tmp = pvector + (i * m);
        // loop on the colums
        for (j; j < m; j++) {

            printf("%5f ", *(pvector_tmp + j)); 
        }
        printf("\n");
        j = 0;
    }
}

int vector_mpy_const(matrix_array* matrix_1, float constant) {


    int answer = 0;
    int columns = matrix_1->columns;
    int rows = matrix_1->rows;
    if (columns == 0) {

        answer = -1;
        return answer;
    }
    else if (rows == 0) {

        answer = -1;
        return answer;
    }

    int m = matrix_1->columns;
    int n = matrix_1->rows;
    int i = 0;
    int j = 0;
    float* pvector = matrix_1->s_vector;
    for (i; i < n; i++) {

        float* pvector_tmp = pvector + (i * m);

        for (j; j < m; j++) {

            *(pvector_tmp + j) = *(pvector_tmp + j) * constant;

        }
        j = 0;
    }
    return answer;
}

int vector_div_const(matrix_array* matrix_1, float constant) {

    int answer = 0;
    int columns = matrix_1->columns;
    int rows = matrix_1->rows;

    if (columns == 0) {

        answer = -1;
        return answer;
    }
    else if (rows == 0) {

        answer = -1;
        return answer;
    }

    int m = matrix_1->columns;
    int n = matrix_1->rows;
    int i = 0;
    int j = 0;
    float* pvector = matrix_1->s_vector;
    for (i; i < n; i++) {

        float* pvector_tmp = pvector + (i * m);

        for (j; j < m; j++) {

            *(pvector_tmp + j) = *(pvector_tmp + j) / constant;

        }
        j = 0;
    }
    return answer;
}
/*mat3 = sum(mat1,mat2)*/
int sum_mat(matrix_array* mat1, matrix_array* mat2, matrix_array* mat3) {

    int answer = 0;
    int columns_1 = mat1->columns;
    int rows_1 = mat1->rows;
    int columns_2 = mat2->columns;
    int rows_2 = mat2->rows;
    if (columns_1 != columns_2) {

        answer = -1;
        mat3->columns = 0;
        mat3->rows = 0;
        return answer;
    }
    else if (rows_1 != rows_2) {

        answer = -1;
        mat3->columns = 0;
        mat3->rows = 0;
        return answer;
    }
    else {

        mat3->columns = mat1->columns;
        mat3->rows = mat1->rows;

    }


    int m = mat1->columns;
    int n = mat1->rows;
    int i = 0;
    int j = 0;
    float* pvector1 = mat1->s_vector;
    float* pvector2 = mat2->s_vector;
    float* pvector3 = mat3->s_vector;
    for (i; i < n; i++) {

        float* pvector_tmp1 = pvector1 + (i * m);
        float* pvector_tmp2 = pvector2 + (i * m);
        float* pvector_tmp3 = pvector3 + (i * m);

        for (j; j < m; j++) {

            *(pvector_tmp3 + j) = *(pvector_tmp1 + j) + *(pvector_tmp2 + j);

        }
        j = 0;
    }

    return answer;
}

/*mat3 = sub(mat1,mat2)*/
int sub_mat(matrix_array* mat1, matrix_array* mat2, matrix_array* mat3) {

    int answer = 0;

    int columns_1 = mat1->columns;
    int rows_1 = mat1->rows;
    int columns_2 = mat2->columns;
    int rows_2 = mat2->rows;
    if (columns_1 != columns_2) {

        answer = -1;
        mat3->columns = 0;
        mat3->rows = 0;
        return answer;
    }
    else if (rows_1 != rows_2) {

        answer = -1;
        mat3->columns = 0;
        mat3->rows = 0;
        return answer;
    }
    else {

        mat3->columns = mat1->columns;
        mat3->rows = mat1->rows;

    }

    int m = mat1->columns;
    int n = mat1->rows;
    int i = 0;
    int j = 0;
    float* pvector1 = mat1->s_vector;
    float* pvector2 = mat2->s_vector;
    float* pvector3 = mat3->s_vector;
    for (i; i < n; i++) {

        float* pvector_tmp1 = pvector1 + (i * m);
        float* pvector_tmp2 = pvector2 + (i * m);
        float* pvector_tmp3 = pvector3 + (i * m);

        for (j; j < m; j++) {

            *(pvector_tmp3 + j) = *(pvector_tmp1 + j) - *(pvector_tmp2 + j);

        }
        j = 0;
    }

    return answer;
}

/* mat3 = mul(mat1,mat2) --------------------------------*/
//
//    K___________      N___________         N___________ 
//   M|           |    K|           |       M|           |
//    | mat1(i,k) |  *  | mat2(k,j) |   =    | mat3(i,j) |
//    |___________|     |___________|        |___________|
//--------------------------------------------------------/
int mul_mat(matrix_array* mat1, matrix_array* mat2, matrix_array* mat3)
{

    int new_columns = 0;
    int new_rows = 0;
    int answer = ret_dim_of_new_matrix(mat1,
                                       mat2,
                                       mat3);
    if (answer == -1) {

        return answer;

    }

    float* A = mat1->s_vector;  int i = 0;  int M = mat1->rows;
    float* B = mat2->s_vector;  int j = 0;  int N = mat2->columns;
    float* C = mat3->s_vector;  int k = 0;  int K = mat2->rows;
                                
    for (i = 0; i < M; ++i)
    {
        float* c = C + i * N;
        for (j = 0; j < N; ++j)
            c[j] = 0;
        for (k = 0; k < K; ++k)
        {
            const float* b = B + k * N;
            float a = A[i * K + k];
            for (j = 0; j < N; ++j)
                c[j] += a * b[j];
        }
    }

    return answer;
}

/* mat3 = row_mul_column(mat1,mat2) --------------------------------*/
//
//    1___________      1___________         1___________ 
//   M|           |    K|           |       1|           |
//    | mat1(i,k) |  *  | mat2(k,j) |   =    | mat3(i,j) |
//    |___________|     |___________|        |___________|
//--------------------------------------------------------/
float row_mul_column(matrix_array* mat1, matrix_array* mat2)
{

    float output_val = 0;
    float* A = mat1->s_vector; 
    int i = 0;  
    float* B = mat2->s_vector;  
    int M = mat1->rows;
    for (i; i < M; ++i)
    {
        output_val = output_val + (*A++) * (*B++);
    }

    return output_val;
}

void copy_mat(matrix_array* mat1, matrix_array* mat2) {

    int m = mat1->columns;
    int n = mat1->rows;
    int i = 0;
    int j = 0;
    float* pvector1 = mat1->s_vector;
    float* pvector2 = mat2->s_vector;
    for (i; i < n; i++) {

        float* pvector_tmp1 = pvector1 + (i * m);
        float* pvector_tmp2 = pvector2 + (i * m);
        for (j; j < m; j++) {

            *(pvector_tmp2 + j) = *(pvector_tmp1 + j);

        }
        j = 0;
    }


}

/*Matrix transpose is an operation on a matrix in which its rows and columns are swapped:
* B[i][j] = A[j][i]
*/
void matrix_transponze(matrix_array* matrix_1, matrix_array* matrix_2) {

    int  M = matrix_1->rows;
    int  N = matrix_1->columns;
    for (int n = 0; n < (M * N); n++) {
        int i = n / N;
        int j = n % N;
        matrix_2->s_vector[n] = matrix_1->s_vector[M * j + i];
    }

}