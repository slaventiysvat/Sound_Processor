#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h >   //Äëÿ memcmp
#include <conio.h>
#include <time.h>
#include "consts_g_729.h"
#include "handler_g_729.h"

//allocating memory for all arrays
short in_m[NUM_OF_SAMPLES] = { 0 }; //16bit values
short in_n[NUM_OF_SAMPLES] = { 0 }; //16bit values
short out[NUM_OF_SAMPLES] = { 0 };  //16bit values

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

	while (1) {


		len_in_m = fread(in_m, 2, NUM_OF_SAMPLES, fp_in_mic_m);//signal from MIC 1  (VOICE signal)
		len_in_n = fread(in_n, 2, NUM_OF_SAMPLES, fp_in_mic_n);//signal from MIC 2  (NOISE signal)

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