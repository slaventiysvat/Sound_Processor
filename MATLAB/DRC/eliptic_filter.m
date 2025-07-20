clear 
clc
close all
%syntize a eliptic filter
[in,Fs] = audioread('..\..\DRC_project\test_sounds\test_sig50_1000Hzt.wav');
Fs_sp = 11025/2;

[b,a] = ellip(2,2,50,200/Fs_sp);

%[b,a] = cheby2(2,50,200/Fs_sp);

%b = [0.0031,0.0061,0.0031];

%a = [1 ,- 1.8801, 0.8924];

a = a * 0.5; %/ max(a);
b = b * 0.5; %/ max(b);

%figure
%impz(a,b,1024);
%figure
%zplane(a,b);
%
%freqz(b,a,2048,11025);
%
%title('n=6 Lowpass Elliptic Filter');
%
%S = filter(b,a,in);


out = zeros(1,length(in));

delay_x1 = 0;
delay_x2 = 0;

delay_y1 = 0;
delay_y2 = 0;
rez = 0;
rez1 = 0;
%y[n] = b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] - a1 * y[n-1] - a2 * y[n-2]
for i = 1:1:length(in)
  
  rez = rez + in(i) * b(1);
  rez = rez + delay_x1 * b(2);
  rez = rez + delay_x2 * b(3);
  
  rez = rez - delay_y1 * a(2);
  rez = rez - delay_y2 * a(3);
  
  out(i) = rez * 1/a(1);
  %out(i) = rez;
  delay_x2 = delay_x1;
  delay_x1  = in(i);
  
  delay_y2 = delay_y1;
  delay_y1  = out(i);
  rez = 0;
  end

%freqz(b,a,2048,11025);

%S = filter(b_norm,a_norm,in);

%sound(S,Fs)
sound(out,Fs)
figure
plot(out)

