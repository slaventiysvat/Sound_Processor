#include "dtd_constants.h"
#include "nlms_handler.h"
#include "math.h"

void dtd_init(dtd_detector_st* dtd_struct,
	          const float Threshould,
	          const float lambda_DTD,
	          const float c,
	          const float alfa) {

	dtd_struct->alfa = alfa;
	dtd_struct->c = c;
	/*dtd_struct->frame_size = SIZE_OF_FRAME(duration_of_frame * samplerate_1);*/
	dtd_struct->frame_size = ceil(duration_of_frame * samplerate);
	dtd_struct->dtd_begin = dtd_struct->frame_size * FIRST_FRAMES;
	dtd_struct->lambda_dtd = lambda_DTD;
	dtd_struct->Threshold = Threshould;
	dtd_struct->number_of_iteration = number_of_iteration;
	dtd_struct->r_em = r_em;
	dtd_struct->length_of_adpt_filter = length_of_adpt_filter;
	dtd_struct->varMIC = varMIC;
	matrix_array* w_coef = matrix_create(length_of_adpt_filter, 1);

	dtd_struct->w = w_coef;
	matrix_array* xin_arr = matrix_create(length_of_adpt_filter, 1);
	dtd_struct->xin = xin_arr;
	matrix_array* xout = matrix_create(dtd_struct->frame_size, 1);
	dtd_struct->xout = xout;
	matrix_array* w_temp_arr = matrix_create(length_of_adpt_filter, 1);
	dtd_struct->w_temp = w_temp_arr;
}

// ------------- NLMS algorithm for Echo Cancelation Adaptive Filter ---------------------- -
void dtd_detector_process(dtd_detector_st* dtd_struct, matrix_array* echo_signal, matrix_array* main_signal) {

	// ------------ NLMS algorithm -----------------------
	int L = dtd_struct->length_of_adpt_filter;
	float T = dtd_struct->Threshold;
	int N = dtd_struct->frame_size;
	float Lambda_DTD = dtd_struct->lambda_dtd;
	float DTDbegin = dtd_struct->dtd_begin;
	//Intial rezult array
	matrix_array* w = dtd_struct->w;
	matrix_array* xin = dtd_struct->xin;
	float varMIC = dtd_struct->varMIC;
	float r_em = dtd_struct->r_em;

	//========The values for calculate Step - Size of Adaptive Filter============
	float alpha = dtd_struct->alfa;
	float c = dtd_struct->c;

	//data
	matrix_array* output_array = dtd_struct->xout;
	int number_of_sample = dtd_struct->number_of_iteration;
	matrix_array* w_temp = dtd_struct->w_temp;

	for (int i = 0; i < N; ++i) {

		//delay line for NLMS filter
		for (int k = L; k >= 1; --k) {

			xin->s_vector[k] = xin->s_vector[k - 1];
		
		}

		xin->s_vector[0] = echo_signal->s_vector[i];                                         // Insert new sample at beginning of input
		output_array->s_vector[i] = row_mul_column(xin, w);                                  // Output signal after adaptive filter
		float error = main_signal->s_vector[i] - output_array->s_vector[i];					 // Estimation error d(i) - y(i)
		float mu = alpha / ( c + row_mul_column(xin, xin) );                                 // Calculate Step - size
		w_temp_calculator(xin, w, w_temp, mu, error);									     // Update filter

		// ---------- - NORMALIZED CROSS - CORELATION ALGORITHM DTD--------------
		if (number_of_sample <= DTDbegin) {
			copy_mat(w_temp, w);
		}

		if (number_of_sample > DTDbegin) {
			// Cross correlation between errorand microphone signal
			r_em = (Lambda_DTD * r_em) + ((1 - Lambda_DTD) * (error * main_signal->s_vector[i]));
			// Variance of microphone signal
			float rez_var = ( Lambda_DTD * (varMIC * varMIC) ) + ( (1 - Lambda_DTD) * ( main_signal->s_vector[i] * main_signal->s_vector[i] ) );
			varMIC = sqrtf(rez_var);
			float decision_statistic = 1 - (r_em / (varMIC * varMIC));                       // Decision statistic
			// Making the double - talk decision
				if (decision_statistic > T) {
					copy_mat(w_temp, w);
				}
		}

		number_of_sample = number_of_sample + 1;
	}

	dtd_struct->varMIC = varMIC;
	dtd_struct->r_em = r_em;

	if (number_of_sample > DTDbegin) {

		number_of_sample = DTDbegin + 1;

	}

	dtd_struct->number_of_iteration = number_of_sample;
}


void w_temp_calculator(matrix_array* xin, matrix_array* w, matrix_array* w_temp, float mu, float error) {

	//wtemp = w + (2 * mu * error * xin); % Update filter
	matrix_array* xin_result = matrix_create(xin->rows, xin->columns);
	float rezult = (2 * mu) * error;
	vector_mpy_const(xin,xin_result,rezult);
	sum_mat(w, xin_result, w_temp);
    matrix_delete(xin_result);
}