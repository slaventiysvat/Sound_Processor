clear all
close all
clc
first_frames = 10;

A = ones(1,320 * first_frames*2)*0.5;
B = ones(1,320 * first_frames*2)*0.2;
x = A;
v = B;
Fs = 16000;                 % Sampling frequency                        
% T = 1/Fs;                   % Sampling period       
% L = 320 * first_frames;     % Length of signal
% t = (0:L-1)*T;              % Time vector
% 
% B = 0.7*sin(2*pi*50*t);
% A = 0.5*sin(2*pi*70*t);

N = length(x);     
Taps = 128;
duration_of_frame = 0.020;                                %ms, durations of frame
size_f = get_size_of_frame(duration_of_frame,Fs);         %size of frame

N_fr = ceil(length(x)/(size_f)); 
%===============================DTD init===================================
Taps_nlms = 128;
%Declare the needed variables
T=0.92; %Threshold for Double talk detection
lambda_DTD=0.95; %Constant for calculating decision statistic of DTD
DTDbegin = first_frames * size_f; %The time to activate DTD
alfa=0.82; %Alfa
c=0.01; %A small constant
dtd_struct = DTD_init(size_f,...
                      Taps_nlms,...      %Length of adaptive filter (same length of RIR)
                      DTDbegin,...       %The time to activate DTD
                      T,...              %Threshold for Double talk detection
                      lambda_DTD,...     %Constant for calculating decision statistic of DTD
                      c,...              %A small constant 'C'
                      alfa);             %Alfa
%=============================train_algorithms=============================
%=============================train_algorithms=============================
signal = [];
vad_answer_arr = [];
%first iteration
head = 1;
tale = size_f;
signal_after_filter = [];
rls_signal = []; 

for i = 1:1:first_frames*2 
    
 noise_signal = x(head:tale);
 desired_signal = v(head:tale);  
 %=================DTD=========================
 [noise_signal,dtd_struct] = double_talk_detector(desired_signal,noise_signal,dtd_struct);
 %================train RLS===================
 signal = [signal  noise_signal'];
 head = head + size_f;
 tale = tale + size_f;
 
end
