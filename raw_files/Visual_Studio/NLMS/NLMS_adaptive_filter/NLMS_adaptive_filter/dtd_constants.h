#ifndef DTD_CONSTANTS
#define DTD_CONSTANTS

#define CEILING(x,y) (((x) + (y) - 1) / (y))
#define DURATION_OF_FRAME      (float)(0.02)
#define SAMPLERATE             (int)(16000)
#define FRAME_SIZE_FILTER      (int)(128)
#define FIRST_FRAMES           (int)(10)
#define FILTER_LENGTH FRAME_SIZE_FILTER 
#define SIZE_OF_FRAME(x) ( ( (x) + (x) - 1) / (x) ) 

extern const float alfa;
extern const float c;
extern const float lambda_dtd;               // Constant for calculating decision statistic of DTD
extern const float Threshold;                // Threshold for Double talk detection
extern const int   dtd_begin;                  // The time to activate DTD
extern const int   frame_size;
extern const int   length_of_adpt_filter;      // Length of adaptive filter(same length of RIR)
extern const int   number_of_iteration;
extern const float varMIC;                   // Initial variance of microphone signal of AF Nx1
extern const float r_em;                     // Initial Cross correlation between errorand microphone signals
extern const float duration_of_frame;
extern const float samplerate;

#endif /*DTD_CONSTANTS*/
