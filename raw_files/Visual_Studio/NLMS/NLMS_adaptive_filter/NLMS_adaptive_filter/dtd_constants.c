#include "dtd_constants.h"

 const float alfa = 0.82f;
 const float c = 0.01f;
 const float lambda_dtd = 0.95f;                            // Constant for calculating decision statistic of DTD
 const float Threshold = 0.92f;                             // Threshold for Double talk detection
 const int frame_size = 0;
 const int dtd_begin = 0;                                   // The time to activate DTD
 const int length_of_adpt_filter = FRAME_SIZE_FILTER;       // Length of adaptive filter(same length of RIR)
 const int number_of_iteration = 0;
 const float varMIC = 0;                                    // Initial variance of microphone signal of AF Nx1
 const float r_em = 0;                                      // Initial Cross correlation between errorand microphone signals
 const float duration_of_frame = DURATION_OF_FRAME;
 const float samplerate = SAMPLERATE;