#ifndef NLMS_HANDLER
#define NLMS_HANDLER

typedef struct dtd_detector_st {

	float alfa;
	float c;
	float lambda_dtd;               // Constant for calculating decision statistic of DTD
	float Threshold;                // Threshold for Double talk detection
	int dtd_begin;                  // The time to activate DTD
	int frame_size;
	int length_of_adpt_filter;      // Length of adaptive filter(same length of RIR)
	int number_of_iteration;
	float varMIC;                   // Initial variance of microphone signal of AF Nx1
	float r_em;                     // Initial Cross correlation between errorand microphone signals
	float* w;                       // Initial weight vector of AF Lx1
	float* xin;                     // Initial input signal of AF Lx1
	
}dtd_detector_st;

void dtd_init(dtd_detector_st* dtd_struct,
	const float Threshould,
	const float lambda_DTD,
	const float c,
	const float alfa);


#endif/*NLMS_HANDLER*/
