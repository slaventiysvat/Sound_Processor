#include "adaptive_filter_rls.h"
#include "defines_values.h"

int rls_init(int p, float lambda, float delta,struct rls_st *rls_st_str) {

	rls_st_str->order = p;
	rls_st_str->lambda = lambda;
	rls_st_str->laminv = 1.0f / lambda;
	rls_st_str->delta = delta;

	rls_st_str->w.colum = 1;
	rls_st_str->w.rows = rls_st_str->order;
	rls_st_str->w.s_vector = memory_allocate1D(rls_st_str->w.colum, rls_st_str->w.rows);// % filter coefficients
	s_zeros(&rls_st_str->w);// = zeros(order, 1);

	rls_st_str->P.colum = rls_st_str->order;
	rls_st_str->P.rows = rls_st_str->order;
	rls_st_str->P.s_vector = memory_allocate1D(rls_st_str->P.colum, rls_st_str->P.rows); // % inverse correlation matrix
	s_eye(&rls_st_str->P, rls_st_str->delta);                                       // = delta_param * eye(order);

	rls_st_str->buff_n.colum = NUM_OF_SAMPLES + rls_st_str->order;
	rls_st_str->buff_n.rows = 1;
	rls_st_str->buff_n.s_vector = memory_allocate1D(rls_st_str->buff_n.colum, rls_st_str->buff_n.rows);
	s_zeros(&rls_st_str->buff_n);// = zeros(order + frame_size, 1);

	rls_st_str->buff_x.colum = NUM_OF_SAMPLES + rls_st_str->order;
	rls_st_str->buff_x.rows = 1;
	rls_st_str->buff_x.s_vector = memory_allocate1D(rls_st_str->buff_x.colum, rls_st_str->buff_x.rows);
	s_zeros(&rls_st_str->buff_x);// = zeros(order + frame_size, 1);

	rls_st_str->error_signal.colum = NUM_OF_SAMPLES + rls_st_str->order;
	rls_st_str->error_signal.rows = 1;
	rls_st_str->error_signal.s_vector = memory_allocate1D(rls_st_str->buff_n.colum, rls_st_str->buff_n.rows);
	s_zeros(&rls_st_str->buff_n);// = zeros(order + frame_size, 1);

	rls_st_str->output_signal.colum = NUM_OF_SAMPLES;
	rls_st_str->output_signal.rows = 1;
	rls_st_str->output_signal.s_vector = memory_allocate1D(rls_st_str->output_signal.colum, rls_st_str->output_signal.rows);
	s_zeros(&rls_st_str->output_signal);// = zeros(frame_size, 1);

	rls_st_str->Pi.colum = 1;
	rls_st_str->Pi.rows = rls_st_str->order;
	rls_st_str->Pi.s_vector = memory_allocate1D(rls_st_str->output_signal.colum, rls_st_str->output_signal.rows);
	s_zeros(&rls_st_str->Pi);// = zeros(frame_size, 1);

	rls_st_str->k.colum = 1;
	rls_st_str->k.rows = rls_st_str->order;
	rls_st_str->k.s_vector = memory_allocate1D(rls_st_str->output_signal.colum, rls_st_str->output_signal.rows);
	s_zeros(&rls_st_str->k);// = zeros(frame_size, 1);

	rls_st_str->y.colum = 1;
	rls_st_str->y.rows = rls_st_str->order;
	rls_st_str->y.s_vector = memory_allocate1D(rls_st_str->output_signal.colum, rls_st_str->output_signal.rows);
	s_zeros(&rls_st_str->y);// = zeros(frame_size, 1);

	return RLS_OK;
}


