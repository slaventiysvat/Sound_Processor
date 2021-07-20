#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h >   //Äëÿ memcmp
//#include <conio.h>
#include <time.h>
#include "nlms_handler.h"
#include "dtd_constants.h"

#define NUM_OF_SAMPLES 320

//allocating memory for all arrays
short in_m[NUM_OF_SAMPLES] = { 0 }; //16bit values
short in_n[NUM_OF_SAMPLES] = { 0 }; //16bit values
short out[NUM_OF_SAMPLES] = { 0 };  //16bit values

dtd_detector_st dtd_settings;

extern const float Threshold;
extern const float lambda_dtd;
extern const float c;
extern const float alfa;

int16_t output_buffer[NUM_OF_SAMPLES];

void float_to_short_cast(matrix_array* casted_array, int16_t* output_buffer,int size);

int main(int argc, char* argv[]) {

	FILE* fp_in_mic_m = fopen("D:/InWite/Sound_Processor/raw_files/alchimia_2_main_mic16_16.raw", "rb");
	FILE* fp_in_mic_n = fopen("D:/InWite/Sound_Processor/raw_files/alchimia_2_noise_mic16_16.raw", "rb");

	FILE* fp_out = fopen("out_result_16b_16kHz.raw", "wb");

	if (!fp_in_mic_m) {

		perror("File opening failed");
		return EXIT_FAILURE;
	}

	if (!fp_in_mic_n) {

		perror("File opening failed");
		return EXIT_FAILURE;

	}

	if (!fp_out) {

		perror("File opening failed");
		return EXIT_FAILURE;

	}

	size_t len_out = 0;
	size_t len_in_m = 0;
	size_t len_in_n = 0;
	
	dtd_init(&dtd_settings, Threshold, lambda_dtd, c, alfa);

	matrix_array* array_voice = matrix_create(NUM_OF_SAMPLES, 1);
	matrix_array* array_noise = matrix_create(NUM_OF_SAMPLES, 1);

	//Test NLMS simple
	//matrix_array* arrayA = matrix_create(NUM_OF_SAMPLES * 20, 1);
	//matrix_array* arrayB = matrix_create(NUM_OF_SAMPLES * 20, 1);
	//matrix_array* arrayC = matrix_create(NUM_OF_SAMPLES * 20, 1);


	//matrix_array* arrayA_tmp = matrix_create(NUM_OF_SAMPLES, 1);
	//matrix_array* arrayB_tmp = matrix_create(NUM_OF_SAMPLES, 1);
	//matrix_array* arrayC_tmp = matrix_create(NUM_OF_SAMPLES, 1);

	//s_values(arrayA, 0.5f);
	//s_values(arrayB, 0.2f);
	///*s_zeros(arrayC);*/


	//int head = 0;
	//int tale = NUM_OF_SAMPLES;
	//int step = NUM_OF_SAMPLES;
	//for (int k = 0; k < 20; ++k) {

	//	for (int d = 0; d < NUM_OF_SAMPLES; ++d) {
	//		arrayA_tmp->s_vector[d] = arrayA->s_vector[d + head];
	//		arrayB_tmp->s_vector[d] = arrayB->s_vector[d + head];
	//	}

	//   dtd_detector_process(&dtd_settings, arrayB_tmp, arrayA_tmp);

	//   for (int d = 0; d < NUM_OF_SAMPLES; ++d) {
	//	   arrayC->s_vector[d + head] = dtd_settings.xout->s_vector[d];
	//   }
 //      head = head + NUM_OF_SAMPLES;
	//}
	   

	while (1) {


		len_in_m = fread(in_m, 2, NUM_OF_SAMPLES, fp_in_mic_m);//signal from MIC 1  (VOICE signal)
		len_in_n = fread(in_n, 2, NUM_OF_SAMPLES, fp_in_mic_n);//signal from MIC 2  (NOISE signal)

		for (int d = 0; d < NUM_OF_SAMPLES; d++) {
			//copy data from input buffers
			array_voice->s_vector[d] = (float)(in_m[d]);
			array_noise->s_vector[d] = (float)(in_n[d]);
		}


		if (len_in_m < NUM_OF_SAMPLES) {

			break;

		}

		if (len_in_n < NUM_OF_SAMPLES) {

			break;

		}

		for (int i = 0; i < NUM_OF_SAMPLES; i++) {

			out[i] = in_n[i];
		}


		dtd_detector_process(&dtd_settings, array_voice, array_noise);

		float_to_short_cast(dtd_settings.xout, output_buffer, NUM_OF_SAMPLES);

		len_out = fwrite(output_buffer, 2, NUM_OF_SAMPLES, fp_out);

	}


	//after break

	short* out_short = (short*)malloc(sizeof(short) * len_in_m);
	clock_t begin = clock();
	len_out = fwrite(out_short, 2, len_in_m, fp_out);
	clock_t end = clock();
	float time_spent = (float)(end - begin) / CLOCKS_PER_SEC;
	printf("spended time %f \n", time_spent);
	printf("%d\n", len_in_m);
	printf("%d\n", len_in_n);
	printf("%d\n", len_out);
	fclose(fp_in_mic_m);
	fclose(fp_in_mic_n);
	fclose(fp_out);

	printf("All is okay");
	//_getch();
	return 0;

}

void float_to_short_cast(matrix_array* casted_array, int16_t* output_buffer,int size) {

	for (int i = 0; i < size; ++i) {
	
		output_buffer[i] = (int16_t)(casted_array->s_vector[i]);
	
	}

}