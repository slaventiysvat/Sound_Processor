function [y,dtd_struct] = double_talk_detector(echo_signal,main_signal,dtd_struct)
%-------------NLMS algorithm for Adaptive Filter-----------------------
L = dtd_struct.L;                           %Length of adaptive filter (same length of RIR)
T = dtd_struct.T;                           %Threshold for Double talk detection
N = dtd_struct.frame_size;                  %Size of frame
lambda_DTD = dtd_struct.lambda_DTD;         %Constant for calculating decision statistic of DTD
DTDbegin = dtd_struct.DTDbegin;             %The time to activate DTD
%Intial rezult array 
w = dtd_struct.w;                           %Initial weight vector of AF Lx1
xin = dtd_struct.xin;                       %Initial input signal of AF Lx1
varMIC = dtd_struct.varMIC;                 %Initial variance of microphone signal of AF Nx1
r_em = dtd_struct.r_em;                     %Initial Cross correlation between error and microphone signals
%=========The values for calculate Step-Size of Adaptive Filter============
alfa = dtd_struct.alfa;                     %Alfa
c = dtd_struct.c;                           %A small constant
%data
x = echo_signal;
d = main_signal;
y = zeros(N,1);
number_of_sample = dtd_struct.number_of_iterations;
for i=1:1:N
    
for j=L:-1:2
 xin(j)=xin(j-1);
end
xin(1)=x(i); %Insert new sample at beginning of input
y(i)=w'*xin; %Output signal after adaptive filter
error = d(i)-y(i); %estimation error
mu=alfa/(c+xin'*xin); %Calculate Step-size
wtemp = w + 2*mu*error*xin;%Update filter
% -----------NORMALIZED CROSS-CORELATION ALGORITHM DTD--------------
if (number_of_sample<=DTDbegin) %The beginning time of the DTD
    w=wtemp; %Update filter coefficients
end
if (number_of_sample > DTDbegin)
%Cross correlation between error and microphone signal
r_em = lambda_DTD * r_em + (1-lambda_DTD)*error*d(i);
%Variance of microphone signal
varMIC = sqrt(lambda_DTD*(varMIC^2)+(1-lambda_DTD)*d(i)*d(i));
decision_statistic = 1 - (r_em / varMIC^2); %Decision statistic
%Making the double-talk decision
if (decision_statistic > T)
    w=wtemp; %Update filter coefficients
end

end
number_of_sample = number_of_sample + 1;
end
dtd_struct.number_of_iterations = number_of_sample;
dtd_struct.w = w;                           %Initial weight vector of AF Lx1
dtd_struct.xin = xin;                       %Initial input signal of AF Lx1
dtd_struct.varMIC = varMIC;                 %Initial variance of microphone signal of AF Nx1
dtd_struct.r_em = r_em;                     %Initial Cross correlation between error and microphone signals
end