clear all
close all
clc 

clear all
close all
clc
%Voice Activity Detector
filename_1 = 'alchimia_2_noise_mic16_16.wav';
filename_2 = 'alchimia_2_main_mic16_16.wav';
[X, Fs] = audioread(filename_1);  %Far-end signal
[V, Fs] = audioread(filename_2);   %Near-end signal
tic
x = X;
v = V;
N = length(x);     
Taps = 128;
duration_of_frame = 0.020;                                %ms, durations of frame
size_f = get_size_of_frame(duration_of_frame,Fs);         %size of frame
first_frames = 10;
N_fr = ceil(length(x)/(size_f)); 
%============================VAD initilization=============================
% constants
Nw  = size_f*2; % size of window for analyze
Nsh = size_f;%    size of frame
decision = [];
VADPar = InitVADPar(Fs,Nw,Nsh,first_frames);
vad_answer = 0;
vad_answer_true = 0;

signal = [];
vad_answer_arr = [];
%first iteration
head = 1;
tale = size_f;
signal_after_filter = [];
rls_signal = []; 

for i = 1:1:first_frames 
    
 noise_signal = x(head:tale);
 desired_signal = v(head:tale);  
 %filtered signal with bypass FIR filter
 %================ VAD ========================
 [vad_answer, VADPar]= VAD(desired_signal, VADPar);
 vad_answer_arr = [vad_answer_arr  vad_answer.*ones(1,size_f)];
 head = head + size_f;
 tale = tale + size_f;
 
end

%========================Main Working Process==============================
first_frames = first_frames + 1;
for i = first_frames:1:N_fr - 2
    
 noise_signal = x(head:tale);
 desired_signal = v(head:tale);  
 %================ VAD ========================
 [vad_answer, VADPar]= VAD(desired_signal, VADPar);
 vad_answer_arr = [vad_answer_arr  vad_answer.*ones(1,size_f)];
 head = head + size_f;
 tale = tale + size_f;

end
toc

figure
plot(v,'b')
hold on
plot(vad_answer_arr,'r')
legend('Original Signal','VAD')




