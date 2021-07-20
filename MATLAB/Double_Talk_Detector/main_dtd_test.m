clear all
close all
clc
%Voice Activity Detector
% filename_1 = 'alchimia_2_noise_mic16_16.wav';
% filename_2 = 'alchimia_2_main_mic16_16.wav';
% [X, Fs] = audioread(filename_1);  %Far-end signal
% [V, Fs] = audioread(filename_2);   %Near-end signal

%prepearing input and output files to processing
filename_1 = 'alchimia_2_noise_mic16_16.raw';
filename_2 = 'alchimia_2_main_mic16_16.raw';
filename_3 = 'alchimia_2_out_mic16_16.raw';

fileID_1 = fopen(filename_1,'r');
fileID_2 = fopen(filename_2,'r');
fileID_3 = fopen(filename_3,'w');

X = fread(fileID_1,'int16');
V = fread(fileID_2,'int16');
Fs = 16000;
tic
x = X;
v = V;
N = length(x);     
Taps = 128;
duration_of_frame = 0.020;                                %ms, durations of frame
size_f = get_size_of_frame(duration_of_frame,Fs);         %size of frame
first_frames = 10;
N_fr = ceil(length(x)/(size_f)); 
%=====================section convolution setting==========================
alfa_nlms=1.2; %Alfa
c_nlms = 0.001;  %Asmall constant
f_1 = 200;
f_2 = 5000;
Taps_nlms = 128;
Pz = filter_coefs_manager(f_1,f_2,Fs,Taps_nlms);
%===============================DTD init===================================
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
 %=================DTD=========================
 [noise_signal,dtd_struct] = double_talk_detector(desired_signal,noise_signal,dtd_struct);
 %================train RLS===================
 signal = [signal  noise_signal'];
 head = head + size_f;
 tale = tale + size_f;
 
end

%========================Main Working Process==============================
first_frames = first_frames + 1;
for i = first_frames:1:N_fr - 2
    
 noise_signal = x(head:tale);
 desired_signal = v(head:tale);  
 %=================DTD=========================
 [noise_signal,dtd_struct] = double_talk_detector(desired_signal,noise_signal,dtd_struct);
 signal = [signal noise_signal'];
 head = head + size_f;
 tale = tale + size_f;

end
toc


% audiowrite('alchimia_4_main_mic16_16_filtred.wav',signal,Fs);

figure
plot(v,'b')
hold on
plot(signal,'r')
legend('Original Signal','Filtred Signal')

%sound(signal,Fs)

fwrite(fileID_3,signal,'int16');

fclose(fileID_1);
fclose(fileID_2);
fclose(fileID_3);


