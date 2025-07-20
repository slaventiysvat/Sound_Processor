clear
clc
%compare three methods to extract envelope of signal
[in,Fs] = audioread('check_DRC_&_NoiseReduction_Clear.wav');

% sound(in,Fs);

PEAK_DET = 0;
RMS_DET = 1;
HILBERT_DET = 0;

if (PEAK_DET == 1)
%-------------------------------peak detector---------------------------------

    attack_time = 0.2;%
    release_time = 40.0;%
    previous_y = 0;%initial value
    out = PEAK(in,attack_time,release_time,Fs,previous_y);
    
    g = gausswin(100); % <-- this value determines the width of the smoothing window
    g = g/sum(g);
    out1 = conv(out, g, 'same');
    
    figure
    subplot(211)
    plot(out)
    legend({'Envelope'},'FontSize',14);
    title('Estimate envelope use PEAK detector');
    %legend({'sin(x)','cos(x)'},'FontSize',12,'TextColor','blue')
    subplot(212)
    plot(out1)
    legend({'Envelope after gauss window'},'FontSize',14);
end

if (RMS_DET == 1)
%-------------------------------RMS detector-----------------------------------
      previous_y  = 0;
      %RMS_detector
      N = 5;
      out = RMS(in,N,previous_y);
      previous_y  = 0;
      N = 15;
      out1 = RMS(in,N,previous_y);
      previous_y  = 0;
      N = 50;
      out2 = RMS(in,N,previous_y);
      figure
      subplot(311)
      plot(out)
      legend({'Envelope with smooth N = 5'},'FontSize',14);
      title('Estimate envelope use RMS detector');
      subplot(312)
      plot(out1)
      legend({'Envelope with smooth N = 15'},'FontSize',14);
      subplot(313)
      plot(out2)
      legend({'Envelope with smooth N = 50'},'FontSize',14);
end

if (HILBERT_DET == 1)
%-------------------------------Hilbert transform detector---------------------

     out = hilbert(in);
     out = abs(out);
     g = gausswin(120); % <-- this value determines the width of the smoothing window
     g = g/sum(g);
     out1 = conv(out, g, 'same');
    
    figure
    subplot(211)
    plot(out)
    legend({'Envelope'},'FontSize',14);
    title('Estimate envelope use HILBERT detector');
    %legend({'sin(x)','cos(x)'},'FontSize',12,'TextColor','blue')
    subplot(212)
    plot(out1)
    legend({'Envelope after gauss window'},'FontSize',14);
    
end
    
figure
plot(in);
hold on
plot(out1,'r','LineWidth',2)
legend('Original Signal','Envelope')
