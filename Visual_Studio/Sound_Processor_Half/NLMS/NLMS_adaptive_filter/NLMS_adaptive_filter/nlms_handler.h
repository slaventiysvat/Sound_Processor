#ifndef NLMS_HANDLER
#define NLMS_HANDLER
#include "../math_lib/special_math.h"
typedef struct dtd_detector_st {

	float alfa;
	float c;
	float lambda_dtd;               // Constant for calculating decision statistic of DTD
	float Threshold;                // Threshold for Double talk detection
	int dtd_begin;                  // The time to activate DTD
	int frame_size;
	int length_of_adpt_filter;      // Length of adaptive filter(same length of RIR)
	int number_of_iteration;
	float varMIC;                   // Initial variance of microphone signal of AF 
	float r_em;                     // Initial Cross correlation between errorand microphone signals
	matrix_array* w;                // Initial weight vector of AF Lx1
	matrix_array* xin;              // Initial input signal of AF Lx1
	matrix_array* xout;             // Output signal of frame_size x 1 (echo was reduced)
	matrix_array* w_temp;           // Temporal filter coefficietns
}dtd_detector_st;

void dtd_init(dtd_detector_st* dtd_struct,
	const float Threshould,
	const float lambda_DTD,
	const float c,
	const float alfa);

void dtd_detector_process(dtd_detector_st* dtd_struct, matrix_array* echo_signal, matrix_array* main_signal);


void w_temp_calculator(matrix_array* xin, matrix_array* w, matrix_array* w_temp, float mu, float error);


#endif/*NLMS_HANDLER*/
