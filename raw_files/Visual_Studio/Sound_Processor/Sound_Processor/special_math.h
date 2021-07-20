#ifndef SPECIAL_MATH
#define SPECIAL_MATH

struct s_matrix_eny {

    int rows;
	int colum;
	float *s_vector;

};

struct s_matrix_eny* s_transponze(struct s_matrix_eny* mat);

/*returns an n - by - m matrix with "val" on the main diagonal and zeros elsewhere.*/
void s_eye(struct s_matrix_eny* mat, float val);

/*returns an n - by - m matrix with "zero values" on the matrix.*/
void s_zeros(struct s_matrix_eny* mat);

/*returns an n - by - m matrix with "one values" on the matrix.*/
void s_ones(struct s_matrix_eny* mat);
/*returns an n - by - m matrix with "values" on the matrix.*/
void s_values(struct s_matrix_eny* mat,float val);

void print_matrix(struct s_matrix_eny* mat);

struct s_matrix_eny* vector_mpy_const(struct s_matrix_eny* vector, float constant);

struct s_matrix_eny* vector_div_const(struct s_matrix_eny* vector, float constant);

struct s_matrix_eny* sum_mat(struct s_matrix_eny* mat1, struct s_matrix_eny* mat2, struct s_matrix_eny* mat3);

struct s_matrix_eny* sub_mat(struct s_matrix_eny* mat1, struct s_matrix_eny* mat2, struct s_matrix_eny* mat3);

struct s_matrix_eny* mul_mat(struct s_matrix_eny* mat1, struct s_matrix_eny* mat2, struct s_matrix_eny* mat3);

float row_mul_column(struct s_matrix_eny* mat1, struct s_matrix_eny* mat2);

void copy_mat(struct s_matrix_eny* mat1, struct s_matrix_eny* mat2);

#endif/*SPECIAL_MATH*/
