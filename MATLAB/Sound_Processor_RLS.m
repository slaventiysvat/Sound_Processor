clear all
close all
clc
%Voice Activity Detector
filename_1 = 'alchimia_2_noise_mic16_16.wav';%MIC 2
filename_2 = 'alchimia_2_main_mic16_16.wav'; %MIC 1
[X, Fs] = audioread(filename_1);  %Far-end signal
[V, Fs] = audioread(filename_2);   %Near-end signal
tic
x = X;
v = V;
N = length(x);     
Taps = 128;
duration_of_frame = 0.020;                                %ms, durations of frame (recommendet to chose 20 or more)
size_f = get_size_of_frame(duration_of_frame,Fs);         %size of frame
first_frames = 10;% how many frames we use to estimating noise (first frames are noise)
N_fr = ceil(length(x)/(size_f)); 
%=====================section convolution setting==========================
alfa_nlms=1.2; %Alfa
c_nlms=0.001; %A small constant
f_1 = 200;
f_2 = 5000;
Taps_nlms = 128;
Pz = filter_coefs_manager(f_1,f_2,Fs,Taps_nlms);
s_conv_st = section_conv_init(Pz,Taps_nlms);
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

%============================VAD initilization=============================
% constants
Nw  = size_f*2; % size of window for analyze
Nsh = size_f;%    size of frame
decision = [];
VADPar = InitVADPar(Fs,Nw,Nsh,first_frames);
vad_answer = 0;
vad_answer_true = 0;
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

for i = 1:1:first_frames 
    
 noise_signal = x(head:tale);
 desired_signal = v(head:tale);  
 %filtered signal with bypass FIR filter
 %================ VAD ========================
 [vad_answer, VADPar]= VAD(desired_signal, VADPar);
 %=================DTD=========================
 [noise_signal,dtd_struct] = double_talk_detector(desired_signal,noise_signal,dtd_struct);
 %================train RLS===================
 [desired_signal,rls_st] = RLSFilterIt(noise_signal,desired_signal,rls_st,vad_answer);
 %filtered signal with bypass FIR filter
 [desired_signal,s_conv_st] = section_conv(desired_signal,s_conv_st);
 signal = [signal  desired_signal'];
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
 %=================DTD=========================
 [noise_signal,dtd_struct] = double_talk_detector(desired_signal,noise_signal,dtd_struct);
 %================train RLS===================
 [desired_signal,rls_st] = RLSFilterIt(noise_signal,desired_signal,rls_st,vad_answer);
  %filtered signal with bypass FIR filter
 [desired_signal,s_conv_st] = section_conv(desired_signal,s_conv_st);
 signal = [signal desired_signal'];
 vad_answer_arr = [vad_answer_arr  vad_answer.*ones(1,size_f)];
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

figure
plot(v,'b')
hold on
plot(vad_answer_arr,'r')
legend('Original Signal','VAD')

sound(signal,Fs)

function s_conv_st = section_conv_init(Wz,size_Wz)
s_conv_st.Wz = Wz;
s_conv_st.size_wz = size_Wz;
s_conv_st.buffer = zeros(size_Wz-1,1);
end

function [conv_rez,s_conv_st] = section_conv(frame,s_conv_st)

 X = [s_conv_st.buffer;frame];
 conv_rez = conv(X,s_conv_st.Wz,'valid');
 s_conv_st.buffer = frame(end - (s_conv_st.size_wz - 2):end);
 
end

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
lambda  = rls_st.lambda;         % forgetting factor
laminv  = rls_st.laminv;
% Filter Initialization
w       = rls_st.w;              % filter coefficients        25x1
P       = rls_st.P;              % inverse correlation matrix 25x25
x = [rls_st.buff_x; x];          %                            345x1 конкатенация буффера и входных массивов
n = [rls_st.buff_n; n];          %                            345x1
rls_st.buff_n = n(end - (p-1):end); %                         25x1
rls_st.buff_x = x(end - (p-1):end); %                         25x1
e       = x*.0;                  % error signal (output)      345x1
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
    rls_st.w = w;                     %25x1
    rls_st.P = P;                     %25x25
    
end


% function rls_st = rls_init_to_c(order,fr_factor,delta_param,frame_size)
% rls_st.p = order;
% rls_st.lambda =  fr_factor;
% rls_st.laminv =  1/fr_factor;
% rls_st.delta  = delta_param;
% rls_st.w = zeros(order,1);             % filter coefficients
% rls_st.P = delta_param*eye(order);     % inverse correlation matrix
% rls_st.buff_n = zeros(order + frame_size,1);        % filter buff noise
% rls_st.buff_x = zeros(order + frame_size,1);        % filter buff signal + noise
% end
% 
% function [e,rls_st] = RLSFilterIt_to_c(n,x,rls_st,vad_dec)
% % n - noise signal
% % x - desired signal (signal + noise)
% %--------------------------------------------------------------------------
% % Filtering
% %--------------------------------------------------------------------------
% % Filter Parameters
% p       = rls_st.p;              % filter order 25
% lambda  = rls_st.lambda;         % forgetting factor
% laminv  = rls_st.laminv;
% % Filter Initialization
% w       = rls_st.w;              % filter coefficients        25x1
% P       = rls_st.P;              % inverse correlation matrix 25x25
% %instead of concatenation of buffers
% indx = length(x) - (p-1);
% for i = 1:1:p
%     rls_st.buff_n(i) = n(indx);
%     rls_st.buff_x(i) = x(indx);
%     indx = indx + 1;
% end
% indx = 1;
% for i = p:1:length(rls_st.buff_x)
%     rls_st.buff_n(i) = n(indx);
%     rls_st.buff_x(i) = x(indx);
%     indx = indx + 1;
% end
% e  = zeros(length(rls_st.buff_x),1);                  % error signal (output)      345x1
% y = zeros(p,1);
%     for m = p:length(x)          %c 25 to 345
%         % Acquire chunk of data
%         %берем кусок массива 25x1 из большого массива в обратном порядке
%         for i = m:-1:m-p+1
%             y(i) = n(i); 
%         end
%         % Error signal equation
%         omega_mul_y = w'*y;
%         e(m) = x(m) - omega_mul_y;             %345x1
%         % Parameters for efficiency
%         if vad_dec == 0
%             Pi = P*y;                 %25x1
%             % Filter gain vector update
%             y_mul_pi = y'*Pi;
%             k = (Pi)/(lambda + y_mul_pi);  %25x1
%             % Inverse correlation matrix update
%             k_mul_y = k*y';
%             p_mul_ky = k_mul_y*P;
%             P = (P - p_mul_ky)*laminv;  %25x25
%             % Filter coefficients adaption
%             w = w + k*e(m);           %25x1
%         end
%     end
%     
%     enew = zeros(length(x),1);
%     indx = p + 1;
%     for i = 1:1:length(x) + p
%         enew(i) = e(indx);
%         indx = indx + 1;
%     end
%     e = enew;
%     rls_st.w = w;                     %25x1
%     rls_st.P = P;                     %25x25
%     
% end



% 
% function rls_st_filt = rls_filt_init(order,w)
% rls_st.p = order;
% rls_st.w = w;             % filter coefficients
% rls_st.buff_n = zeros(order,1);        % filter buff noise
% rls_st.buff_x = zeros(order,1);        % filter buff signal + noise
% end
% 
% function [e,rls_st_filt] = RLSFilt(n,x,rls_st_filt)
% % n - noise signal
% % x - desired signal (signal + noise)
% %--------------------------------------------------------------------------
% % Filtering
% %--------------------------------------------------------------------------
% % Filter Parameters
% p       = rls_st_filt.p;              % filter order 64
% % Filter Initialization
% w       = rls_st_filt.w;              % filter coefficients
% x = [rls_st.buff_x; x];
% n = [rls_st.buff_n; n];
% rls_st.buff_n = n(end - (p-1):end);
% rls_st.buff_x = x(end - (p-1):end);
% e       = x*.0;                  % error signal (output)
% 
%     for m = p:length(x)
%         % Acquire chunk of data
%         y = n(m:-1:m-p+1);
%         % Error signal equation
%         e(m) = x(m)-w'*y;
%     end
%     e = e(rls_st.p+1:end);
% end


