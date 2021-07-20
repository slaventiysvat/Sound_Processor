#include "special_math.h"
#include "stdio.h"

struct s_matrix_eny* s_transponze(struct s_matrix_eny* mat) {

    int new_colum = mat->rows;
    int new_rows = mat->colum;
    mat->colum = new_colum;
    mat->rows = new_rows;
    return mat;

}

/*returns an n - by - m matrix with "val" on the main diagonal and zeros elsewhere.*/
void s_eye(struct s_matrix_eny* mat, float val) {
    int m = mat->colum;
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
void s_zeros(struct s_matrix_eny* mat) {
    int m = mat->colum;
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
void s_ones(struct s_matrix_eny* mat) {
    int m = mat->colum;
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

void s_values(struct s_matrix_eny* mat, float val) {

    int m = mat->colum;
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

void print_matrix(struct s_matrix_eny* mat) {
    int m = mat->colum;
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

struct s_matrix_eny* vector_mpy_const(struct s_matrix_eny* mat, float constant) {

    int m = mat->colum;
    int n = mat->rows;
    int i = 0;
    int j = 0;
    float* pvector = mat->s_vector;
    for (i; i < n; i++) {

        float* pvector_tmp = pvector + (i * m);

        for (j; j < m; j++) {

            *(pvector_tmp + j) = *(pvector_tmp + j) * constant;

        }
        j = 0;
    }
    return mat;
}

struct s_matrix_eny* vector_div_const(struct s_matrix_eny* mat, float constant) {

    int m = mat->colum;
    int n = mat->rows;
    int i = 0;
    int j = 0;
    float* pvector = mat->s_vector;
    for (i; i < n; i++) {

        float* pvector_tmp = pvector + (i * m);

        for (j; j < m; j++) {

            *(pvector_tmp + j) = *(pvector_tmp + j) / constant;

        }
        j = 0;
    }
    return mat;
}
/*mat3 = sum(mat1,mat2)*/
struct s_matrix_eny* sum_mat(struct s_matrix_eny* mat1, struct s_matrix_eny* mat2, struct s_matrix_eny* mat3) {

    int m = mat1->colum;
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

    return mat3;
}

/*mat3 = sub(mat1,mat2)*/
struct s_matrix_eny* sub_mat(struct s_matrix_eny* mat1, struct s_matrix_eny* mat2, struct s_matrix_eny* mat3) {

    int m = mat1->colum;
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

    return mat3;
}

/* mat3 = mul(mat1,mat2) --------------------------------*/
//
//    K___________      N___________         N___________ 
//   M|           |    K|           |       M|           |
//    | mat1(i,k) |  *  | mat2(k,j) |   =    | mat3(i,j) |
//    |___________|     |___________|        |___________|
//--------------------------------------------------------/
struct s_matrix_eny* mul_mat(struct s_matrix_eny* mat1, struct s_matrix_eny* mat2, struct s_matrix_eny* mat3)
{

    mat3->colum = mat1->rows;
    mat3->rows = mat2->colum;

    float* A = mat1->s_vector;  int i = 0;  int M = mat3->colum;
    float* B = mat2->s_vector;  int j = 0;  int N = mat3->rows;
    float* C = mat3->s_vector;  int k = 0;  int K = mat1->colum;
                                
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

    return mat3;
}

/* mat3 = row_mul_column(mat1,mat2) --------------------------------*/
//
//    1___________      1___________         1___________ 
//   M|           |    K|           |       1|           |
//    | mat1(i,k) |  *  | mat2(k,j) |   =    | mat3(i,j) |
//    |___________|     |___________|        |___________|
//--------------------------------------------------------/
float row_mul_column(struct s_matrix_eny* mat1, struct s_matrix_eny* mat2)
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

void copy_mat(struct s_matrix_eny* mat1, struct s_matrix_eny* mat2) {


    int m = mat1->colum;
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