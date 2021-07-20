#ifndef ADAPTIVE_FILTER_RLS
#define ADAPTIVE_FILTER_RLS

#define RLS_OK (int)(0);
#include "special_math.h"
#include "s_malloc.h"

typedef struct rls_st {
	// Filter constant Parameters
	int order;        //filter order
	float lambda;     //forgetting factor
	float laminv;
	float delta;
	//Filter main tmp buffers
	matrix_array* w;        // = zeros(order, 1);% filter coefficients
	matrix_array* P;        // = delta_param * eye(order); % inverse correlation matrix
	matrix_array* buff_n;   // = zeros(order, 1);% filter buff noise
	matrix_array* buff_x;   // = zeros(order, 1); % filter buff signal + noise
	matrix_array* x;        // = zeros(order  + frame_size, 1);% filter buff noise
	matrix_array* n;        // = zeros((order + frame_size, 1); % filter buff signal + noise
	matrix_array* error_signal;
	matrix_array* output_signal;
	//Filter other tmp buffers
	matrix_array* Pi;
	matrix_array* k; //Filter gain vector
	matrix_array* y; //for chunk of data
}rls_st;

int rls_init(int p, float lambda, float delta, struct rls_st *rls_st_str);

//struct rls_st *rls_filterIt(struct matrix_array*noise_signal,
//							struct matrix_array*desired_signal,
//							struct rls_st* rls_st_str,
//							int vad_dec);

void rls_filter_process(rls_st* rls_st_str,
	                    matrix_array* noise_signal, 
	                    matrix_array* desired_signal,
	                    int vad_answer);

#endif/*ADAPTIVE_FILTER_RLS*/
