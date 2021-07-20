#include "dtd_constants.h"
#include "nlms_handler.h"

void dtd_init(dtd_detector_st* dtd_struct,
	          const float Threshould,
	          const float lambda_DTD,
	          const float c,
	          const float alfa) {

	dtd_struct->alfa = alfa;
	dtd_struct->c = c;
	dtd_struct->frame_size = SIZE_OF_FRAME(duration_of_frame * samplerate);
	dtd_struct->dtd_begin = dtd_struct->frame_size * FIRST_FRAMES;
	dtd_struct->lambda_dtd = lambda_DTD;
	dtd_struct->Threshold = Threshould;
	dtd_struct->number_of_iteration = number_of_iteration;
	dtd_struct->r_em = r_em;
	dtd_struct->length_of_adpt_filter = length_of_adpt_filter;
	dtd_struct->varMIC = varMIC;
	dtd_struct->w = 
	

}
