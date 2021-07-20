#ifndef ADAPTIVE_FILTER_RLS
#define ADAPTIVE_FILTER_RLS

#define RLS_OK (int)(0);

#include "../vector_math/special_math.h"
#include "../vector_math/s_malloc.h"

struct rls_st {
	// Filter constant Parameters
	int order;        //filter order
	float lambda;     //forgetting factor
	float laminv;
	float delta;
	//Filter main tmp buffers
	struct s_matrix_eny w;        // = zeros(order, 1);% filter coefficients
	struct s_matrix_eny P;        // = delta_param * eye(order); % inverse correlation matrix
	struct s_matrix_eny buff_n;   // = zeros(order, 1);% filter buff noise
	struct s_matrix_eny buff_x;   //= zeros(order, 1); % filter buff signal + noise
	struct s_matrix_eny error_signal;
	struct s_matrix_eny output_signal;
	//Filter other tmp buffers
	struct s_matrix_eny Pi; 
	struct s_matrix_eny k; //Filter gain vector
	struct s_matrix_eny y; //for chunk of data
};

int rls_init(int p, float lambda, float delta, struct rls_st *rls_st_str);

struct rls_st *rls_filterIt(struct s_matrix_eny *noise_signal,
							struct s_matrix_eny *desired_signal, 
							struct rls_st* rls_st_str,
							int vad_dec);

#endif/*ADAPTIVE_FILTER_RLS*/
