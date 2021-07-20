#include "adaptive_filter_rls.h"
#include "defines_values.h"
int rls_init(int p, float lambda, float delta,struct rls_st *rls_st_str) {

	rls_st_str->order = p;
	rls_st_str->lambda = lambda;
	rls_st_str->laminv = 1.0f / lambda;
	rls_st_str->delta = delta;

	rls_st_str->w = matrix_create(rls_st_str->order, 1);                             //  filter coefficients

	rls_st_str->P = matrix_create(rls_st_str->order, rls_st_str->order);             //  inverse correlation matrix
	s_eye(rls_st_str->P, rls_st_str->delta);                                         //= delta_param * eye(order);

	rls_st_str->buff_n = matrix_create(rls_st_str->order, 1);                        // = zeros(order + frame_size, 1);
	rls_st_str->buff_x = matrix_create(rls_st_str->order, 1);                        // = zeros(order + frame_size, 1);

	rls_st_str->n = matrix_create(NUM_OF_SAMPLES + rls_st_str->order, 1);            // = zeros(order + frame_size, 1);
	rls_st_str->x = matrix_create(NUM_OF_SAMPLES + rls_st_str->order, 1);            // = zeros(order + frame_size, 1);

	rls_st_str->output_signal = matrix_create(NUM_OF_SAMPLES, 1);                    // = zeros(frame_size, 1);

	rls_st_str->Pi = matrix_create(rls_st_str->order, 1);                            // = zeros(order, 1);
	rls_st_str->k = matrix_create(rls_st_str->order, 1);                             // = zeros(order, 1);
	rls_st_str->y = matrix_create(rls_st_str->order, 1);                             // = zeros(order, 1);

	return RLS_OK;
}


void rls_filter_process(rls_st* rls_st_str, matrix_array* noise_signal, matrix_array* desired_signal, int vad_answer) {

	//Filter Parameters
	matrix_array* w = rls_st_str->w;
	matrix_array* P = rls_st_str->P;
	matrix_array* Pi = rls_st_str->Pi;
	matrix_array* k = rls_st_str->k;
	matrix_array* y = rls_st_str->y;
	matrix_array* buff_n = rls_st_str->buff_n;
	matrix_array* buff_x = rls_st_str->buff_x;
	matrix_array* x = rls_st_str->x;
	matrix_array* n = rls_st_str->n;
	matrix_array* output_signal = rls_st_str->output_signal;
	
	int order = rls_st_str->order;
	float lambda = rls_st_str->lambda;
	float laminv = rls_st_str->laminv;

	// Filter Initialization
	float* n_s_end = noise_signal->s_vector + ((NUM_OF_SAMPLES - order));
	float* d_s_end = desired_signal->s_vector + ((NUM_OF_SAMPLES - order));

	for (int i = 0; i < order; ++i) {
	
		n->s_vector[i] = buff_n->s_vector[i];
		buff_n->s_vector[i] = *n_s_end++;
		x->s_vector[i] = buff_x->s_vector[i];
		buff_x->s_vector[i] = *d_s_end++;

	}

	int k_it = 0;
	for (int i = order; i < (NUM_OF_SAMPLES + order); ++i) {

		n->s_vector[i] = noise_signal->s_vector[k_it];
		x->s_vector[i] = desired_signal->s_vector[k_it];
		k_it = k_it + 1;

	}

	s_zeros(output_signal);										//error signal(output)      (NUM_OF_SAMPLES + order)x1
	matrix_array* P_tmp = matrix_create(order, order);
	matrix_array* k_mul_y = matrix_create(order, order);
	matrix_array* y_tr = matrix_create(1, order);

	//from order to (NUM_OF_SAMPLES + order)
	int iter = 0;
	for (int m = order-1; m < NUM_OF_SAMPLES + order; ++m) {
		//Acquire chunk of data
		int d = 0;
		for (int i = m; i > (m -(order - 1)); --i) {
		//Back copy for "order" samples
			y->s_vector[d] = n->s_vector[i];
			d = d + 1;
		}
		//Error signal equation
		output_signal->s_vector[iter] = x->s_vector[m] - row_mul_column(w, y);
		//Parameters for efficiency
		if (vad_answer == 0) {
		
			int state = mul_mat(P, y, Pi);
			//Filter gain vector update
			float tmp = lambda + row_mul_column(y, Pi);
			state = vector_div_const(Pi, k, tmp);
			//Inverse correlation matrix update
		    matrix_transponze(y, y_tr);
			state = mul_mat(k, y_tr, k_mul_y);
			state = mul_mat(k_mul_y, P, P_tmp);
			state = vector_mpy_const(P_tmp, P_tmp, laminv);
			state = vector_mpy_const(P, P, laminv);
			state = sub_mat(P, P_tmp, P);
			//Filter coefficients adaption
			state = vector_mpy_const(k, k, output_signal->s_vector[iter]);
			state = sum_mat(w, k, w);
		
		}
		iter = iter + 1;
	}

	matrix_delete(P_tmp);
	matrix_delete(k_mul_y);
	matrix_delete(y_tr);
}