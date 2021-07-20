clear all
close all
clc
%Voice Activity Detector
% filename_1 = 'alchimia_2_noise_mic16_16.wav';%MIC 2
% filename_2 = 'alchimia_2_main_mic16_16.wav'; %MIC 1
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

tic
Fs = 16000;
x = X;
v = V;
N = length(x);     
Taps = 128;
duration_of_frame = 0.020;                                %ms, durations of frame (recommendet to chose 20 or more)
size_f = get_size_of_frame(duration_of_frame,Fs);         %size of frame
first_frames = 10;% how many frames we use to estimating noise (first frames are noise)
N_fr = ceil(length(x)/(size_f)); 

%============================RLS initilization=============================
order = 25;
fr_factor = 1.85;
delta_param = 0.001;
rls_st = rls_init(order,fr_factor,delta_param);
%=============================train_algorithms=============================
signal = [];
vad_answer_arr = [];
%first iteration
head = 1;
tale = size_f;
signal_after_filter = [];
rls_signal = []; 
vad_answer = 0;
for i = 1:1:first_frames 
    
 noise_signal = x(head:tale);
 desired_signal = v(head:tale);  
 %================train RLS===================
 [desired_signal,rls_st] = RLSFilterIt(noise_signal,desired_signal,rls_st,vad_answer);
 signal = [signal  desired_signal'];
 head = head + size_f;
 tale = tale + size_f;
 
end

%========================Main Working Process==============================
first_frames = first_frames + 1;
for i = first_frames:1:N_fr - 2
    
 noise_signal = x(head:tale);
 desired_signal = v(head:tale);  
 [desired_signal,rls_st] = RLSFilterIt(noise_signal,desired_signal,rls_st,vad_answer);
 signal = [signal desired_signal'];
 head = head + size_f;
 tale = tale + size_f;

end
toc

figure
plot(v,'b')
hold on
plot(signal,'r')
legend('Original Signal','Filtred Signal')

sound(signal,Fs)

fwrite(fileID_3,signal,'int16');

fclose(fileID_1);
fclose(fileID_2);
fclose(fileID_3);


function rls_st = rls_init(order,fr_factor,delta_param)
rls_st.p = order;
rls_st.lambda =  fr_factor;
rls_st.laminv =  1/fr_factor;
rls_st.delta  = delta_param;
rls_st.w = zeros(order,1);             % filter coefficients
rls_st.P = delta_param*eye(order);     % inverse correlation matrix
rls_st.buff_n = zeros(order,1);        % filter buff noise
rls_st.buff_x = zeros(order,1);        % filter buff signal + noise
end

function [e,rls_st] = RLSFilterIt(n,x,rls_st,vad_dec)
% n - noise signal
% x - desired signal (signal + noise)
%--------------------------------------------------------------------------
% Filtering
%--------------------------------------------------------------------------
% Filter Parameters
p       = rls_st.p;              % filter order 25
lambda  = cast(rls_st.lambda,'single');         % forgetting factor
laminv  = cast(rls_st.laminv,'single');
% Filter Initialization
w       = cast(rls_st.w,'single');              % filter coefficients        25x1
P       = cast(rls_st.P,'single');              % inverse correlation matrix 25x25
x = cast([rls_st.buff_x; x],'single');          %                            345x1 конкатенация буффера и входных массивов
n = cast([rls_st.buff_n; n],'single');          %                            345x1
rls_st.buff_n = cast(n(end - (p-1):end),'single'); %                         25x1
rls_st.buff_x = cast(x(end - (p-1):end),'single'); %                         25x1
e       = cast(x*.0,'single');                  % error signal (output)      345x1
    for m = p:length(x)          %c 25 to 345
        % Acquire chunk of data
        y = n(m:-1:m-p+1);       %берем кусок массива 25x1 из большого массива в обратном порядке
        % Error signal equation
        e(m) = x(m)-w'*y;             %345x1 
        % Parameters for efficiency
        if vad_dec == 0
            Pi = P*y;                 %25x1
            % Filter gain vector update
            k = (Pi)/(lambda+y'*Pi);  %25x1
            % Inverse correlation matrix update
            P = (P - k*y'*P)*laminv;  %25x25
            % Filter coefficients adaption
            w = w + k*e(m);           %25x1
        end
    end
    
    e = e(rls_st.p+1:end);            %320x1
    rls_st.w = cast(w,'single');                     %25x1
    rls_st.P = cast(P,'single');                     %25x25
    
end


