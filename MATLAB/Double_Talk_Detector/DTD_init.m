function dtd_struct = DTD_init(frame_size,adaptive_length,time_to_activate,treshould,lambda_DTD,C,alpha)
dtd_struct.frame_size = frame_size;
dtd_struct.L = adaptive_length; %Length of adaptive filter (same length of RIR)
dtd_struct.T = treshould; %Threshold for Double talk detection
dtd_struct.lambda_DTD = lambda_DTD; %Constant for calculating decision statistic of DTD
dtd_struct.DTDbegin = time_to_activate; %The time to activate DTD
%Intial value 0
dtd_struct.w = zeros(adaptive_length,1); %Initial weight vector of AF Lx1
dtd_struct.xin = zeros(adaptive_length,1); %Initial input signal of AF Lx1
dtd_struct.varMIC = 0; %Initial variance of microphone signal of AF Nx1
dtd_struct.r_em = 0; %Initial Cross correlation between error and microphone signals
%=========The values for calculate Step-Size of Adaptive Filter============
dtd_struct.alfa = alpha; %Alfa
dtd_struct.c = C; %A small constant
dtd_struct.number_of_iterations = 0;
end