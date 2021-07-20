#ifndef CONSTS_G_729
#define CONSTS_G_729

#define NUM_OF_SAMPLES 128

extern const int BWExp;        // 60 Hz bandwidth expansion, Gaussian window
extern const int N0;           // number of frames for long - term min energy calculation
extern const int INIT_COUNT;   // Number of samples based on which VAD will adapt 
extern const int Init_Noise;   // Initial Noise Threshold
extern const int M;            // LP (Linear Prediction) order
extern const int NP;           // autocorrelation order
extern const int Fs;           // Current Samplerate
extern const float pi;         // Constant of Number Pi
//LP analysis, lag window applied to autocorrelation coefficients
extern const float w0;         // Constant for calculating the window for the linear predictor
extern const float Wn;         // White noise compensation (diagonal loading)

//initialize variable parameters
extern const int FrmCount = 0;
extern const float *FrmEn;//VADPar.FrmEn = Inf * ones(1,VADPar.N0);
extern const float *MeanLSF;//VADPar.MeanLSF = zeros(VADPar.M, 1);
extern const float MeanSE = 0;
extern const float MeanSLE = 0;
extern const float MeanE = 0;
extern const float MeanSZC = 0;
extern const int count_sil = 0;
extern const int count_inert = 0; //% modified for AppendixII
extern const int count_update = 0;
extern const int count_ext = 0;
extern const int less_count = 0;
extern const int flag = 1;

extern const int *PrevMarkers;//VADPar.PrevMarkers = [1, 1];
extern const float PrevEnergy = 0;

extern const float Prev_MinE = Inf;
extern const float Next_MinE = Inf;
extern const float *MinE_buffer;//VADPar.MinE_buffer = Inf * ones(1, VADPar.N0/8);

#endif /*CONSTS_G_729*/
