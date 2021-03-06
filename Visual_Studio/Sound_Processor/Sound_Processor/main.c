#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h >   //??? memcmp
#include <conio.h>
#include <time.h>
#include "defines_values.h"
#include "special_math.h"
#include "s_malloc.h"
#include "adaptive_filter_rls.h"
//allocating memory for all arrays
short in_m[NUM_OF_SAMPLES] = { 0 }; //16bit values
short in_n[NUM_OF_SAMPLES] = { 0 }; //16bit values
short out[NUM_OF_SAMPLES] = { 0 };  //16bit values

//void matrix_mpy(struct s_matrix_eny *mat1, struct s_matrix_eny *mat2, struct s_matrix_eny *mat3);

int main(int argc, char* argv[]) {

	FILE* fp_in_mic_m = fopen("D:/InWite/Sound_Processor/raw_files/alchimia_2_main_mic16_16.raw", "rb");
	FILE* fp_in_mic_n = fopen("D:/InWite/Sound_Processor/raw_files/alchimia_2_noise_mic16_16.raw", "rb");

	FILE* fp_out = fopen("out.raw", "wb");

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

	int tst_val_size = NUM_OF_SAMPLES;

	//struct rls_st rls_struct;
	//int p = 25;
	//float lambda = 1.85;
	//float delta = 0.001;
	//struct rls_st rls_st_str;
	//int state = rls_init(p, lambda, delta, &rls_st_str);

	while (1) {
		

		len_in_m = fread(in_m, 2, NUM_OF_SAMPLES, fp_in_mic_m);
		len_in_n = fread(in_n, 2, NUM_OF_SAMPLES, fp_in_mic_n);

		if (len_in_m < NUM_OF_SAMPLES) {

			break;

		}

		if (len_in_n < NUM_OF_SAMPLES) {

			break;

		}
	
		for (int i = 0; i < NUM_OF_SAMPLES; i++) {

			out[i] = in_n[i];
		}

		

		len_out = fwrite(out, 2, NUM_OF_SAMPLES, fp_out);

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
	_getch();
	return 0;

}

struct rls_st* rls_filterIt(struct s_matrix_eny* noise_signal,
	struct s_matrix_eny* desired_signal,
	struct rls_st* rls_st_str,
	int vad_dec) {
	//// --------------------------------------------------------------------------
 //   //Filtering
	//// --------------------------------------------------------------------------
	////Filter Parameters
	//int p = rls_st_str->order;
	//float lambda = rls_st_str->lambda;
	//float laminv = rls_st_str->laminv;
	////Filter buffers variable
	//struct s_matrix_eny w = rls_st_str->w;
	//struct s_matrix_eny *P = &rls_st_str->P;
	//struct s_matrix_eny *Pi = &rls_st_str->Pi;
	//struct s_matrix_eny *k = &rls_st_str->k;
	//int d = p;
	//for (d; d < NUM_OF_SAMPLES; d++) {
	//
	//	//% Acquire chunk of data
	//	int m = d;
	//	int k = d - (p + 1);
	//	int b = 0;
	//	for (m; m > k; m--) {
	//	
	//		rls_st_str->y.s_vector[b] = rls_st_str->buff_n.s_vector[m];
	//		b = b + 1;
	//	}
	//	//% Error signal equation
	//	rls_st_str->error_signal.s_vector[d] = rls_st_str->error_signal.s_vector[d] - row_mul_column(&w, &rls_st_str->y);
	//	//% Parameters for efficiency
	//	if (vad_dec == 0) {
	//	    Pi = mul_mat(&P, &rls_st_str->y, &Pi);
	//	   //% Filter gain vector update
	//		float lambda_new = lambda + row_mul_column(&rls_st_str->y, &Pi);
	//		k = vector_div_const(&Pi, lambda_new);
	//	   //% Inverse correlation matrix update
	//		float mul_rez = row_mul_column(&k, &rls_st_str->y);
	//		/*float laminv_mul = laminv **/ 
	//		/*P = vector_mpy_const(&P, laminv);*/
	//	
	//	   //% Filter coefficients adaption

	//	}
	//
	//
	//}

	return rls_st_str;
}