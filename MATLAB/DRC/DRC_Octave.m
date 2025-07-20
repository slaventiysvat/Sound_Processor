clear 
clc
close all
%choosing method of estimating envelope
PEAK_DET = 1;
RMS_DET = 0;%works incorect in Octave
%--------------------------------------
NT = 1;
LT = 90;
EX = 10;
CT = 70;

CT_N =  0.01 * NT;
CT_LT = 0.01 * LT;
CT_EX = 0.01 * EX;
CT_CT = 0.01 * CT;

gain = 300;

threshoultLT = CT_LT;
threshoultNT = CT_N;
threshoultEX = CT_EX;
threshoultCT = CT_CT;

%alfaNG = (0* pi) / 180;%1
%alfaEX = (15* pi) / 180;
%alfaCM = (150* pi) / 180;
%alfaLT = (90* pi) / 180;
%alfaGT = (90* pi) / 180;

alfaNG = (0* pi) / 180;%1
alfaEX = (16* pi) / 180;
alfaCM = (65* pi) / 180;
alfaLT = (60* pi) / 180;
alfaGT = (60* pi) / 180;

%alfaNG = (45* pi) / 180;%1
%alfaEX = (45* pi) / 180;
%alfaCM = (45* pi) / 180;
%alfaLT = (45* pi) / 180;
%alfaGT = (45* pi) / 180;


y = zeros(1,101);
x = 0:0.01:1;
y = x;
%----------------------------static curve---------------------------------------
%static curve for example
if (alfaNG > 0) k_ng = 1/tan(alfaNG); b_ng = (1  - k_ng) * CT_N;  else k_ng = 0; b_ng = 0; end

if (alfaEX > 0) k_ex = 1/tan(alfaEX); b_ex = (1  - k_ex) * CT_N;  else k_ex = 0; b_ex = 0; end

if (alfaCM > 0) k_cm = 1/tan(alfaCM); b_cm = (1  - k_cm) * CT_LT; else k_cm = 0; b_cm = 0; end

if (alfaLT > 0) k_lt = 1/tan(alfaLT); b_lt = (1  - k_lt) * CT_LT; else k_lt = 0; b_lt = 0; end

if (alfaGT > 0) k_gt = 1/tan(alfaGT); b_gt = (1  - k_gt) * CT_EX; else k_gt = 0; b_gt = 0; end

%noise gate
for i=1:1:101
  
  %noise gate
  if (y(i) < CT_N)
    
     y(i) = (k_ng * x(i)) + (x(i) * b_ng);
    
     if (y(i) < 0) y(i) = 0;  end
    
 %expander
 elseif ((y(i) > CT_N) && (y(i) <= CT_EX))
  
     y(i) = (k_ex *(x(i))) + (x(i) *b_ex);
   
     if (y(i) < 0) y(i) = 0;  end
    
 %straight curve
  elseif  ((y(i) > CT_EX - 3) && (y(i) <= CT_CT))

     y(i) = ((k_gt * x(i)) + (x(i) * b_gt))*(gain * 0.01);

     if (y(i) < 0) y(i) = 0;  end
       
 %compressor
  elseif ((y(i) > CT_CT-3) && (y(i) <= CT_LT))
   
     y(i) = (k_cm * x(i)) + (x(i) *b_cm);
  
     if (y(i) < 0) y(i) = 0;  end
  
 %Limiter
 elseif ((y(i) > CT_LT-3))
 
     y(i) = (k_lt * (x(i))) + b_lt;
  
     if (y(i) < 0) y(i) = 0;  end
  
 end
 
 
end


figure
x1 = 0:0.01:1;
plot(x1)
hold on
plot(y)
ylim([0 1])
legend('simple curve','modified curve');

[in,Fs] = audioread('check32bit.wav');
S = in;
if (PEAK_DET == 1)
%-------------------------------peak detector-----------------------------------

    attack_time = 10.0;%
    release_time = 80.0;%
    AT = (-2.2 *(1.0 / Fs)) / (attack_time / 1000.0);
    RT = (-2.2 *(1.0 / Fs)) / (release_time / 1000.0);
    AT = 1.0 - exp(AT);
    RT = 1.0 - exp(RT);
    previous_y = 0;%initial value
    in = S;
    out = zeros(1,length(in));

    if (abs(in(1)) > previous_y) 

      out(1) = (1 - AT) * previous_y + abs(in(1))*AT;

    end

    if (abs(in(1)) <= previous_y) 

      out(1) = (1 - RT) * previous_y;
        
    end

    for i = 2:1:length(in)

           
        if (abs(in(i)) > out(i - 1)) 

          out(i) = (1 - AT) * out(i - 1) + abs(in(i))*AT;

            end
            
        if (abs(in(i)) <= out(i - 1)) 

          out(i) = (1 - RT) * out(i - 1);

         end

  end  
  
   %out = hilbert(in);
   %out= abs(out);
    
    figure
    plot(S);
    hold on
    plot(out,'r','LineWidth',2)
    legend('Original Signal','Envelope')
    
end





if (RMS_DET == 1)
%-------------------------------RMS detector------------------------------------
      previous_y  = 0;

      %RMS_detector
      out = zeros(1,length(in));

      N = 100;

      out(1) = (previous_y * ((N - 1.0) / N)) + ((in(1)^2) * (1.0 / N));

      for i = 2:1:length(in)
       
        out(i) = (out(i - 1) * ((N - 1.0) / N)) + ((in(i)^2) * (1.0 / N));

      end

      for i = 1:1:length(in)

          out(i) = sqrt(out(i));

      end

      figure
      plot(in)
      hold on
      plot(out,'r')
      hold off
      title('RMS detector')
      legend('Original Signal','Envelope')
end


%-------------------Noise gate/expander/compressor------------------------------

new_dble = zeros(1,length(in));

for i = 1:1:length(in)
  
  %noise gate
  if (out(i) <= threshoultNT)
    
     new_dble(i) = (k_ng * out(i)) - b_ng;
    
     if (new_dble(i) < 0) new_dble(i) = 0;  end
    
  %expander
  elseif((out(i) > threshoultNT) && (out(i) <= threshoultEX))
  
     new_dble(i) =  (k_ex *(out(i))) - b_ex;
   
     if (new_dble(i) < 0) new_dble(i) = 0;  end

  %straight curve * gain
  elseif ((out(i) > (threshoultEX + 0.01)) && (out(i) <= threshoultCT))
  
     new_dble(i) =  ((k_gt * out(i)) - b_gt)*(gain * 0.01);

     if (new_dble(i) < 0) new_dble(i) = 0;  end;
    
  %compressor
  elseif ((out(i) > (threshoultCT+ 0.01)) && (out(i) <= threshoultLT))
  
     new_dble(i) = (k_cm * out(i)) - b_cm;
   
     if (new_dble(i) < 0) new_dble(i) = 0;  end
   
  %Limiter
  elseif (out(i) > threshoultLT)
   
     new_dble(i) = (k_lt * (out(i))) - b_lt;
	 
	 if (new_dble(i) < 0) new_dble(i) = 0;  end
     
  else 
     
     new_dble(i) = out(i);
     
  end
  
 
end

new = zeros(1,length(in));

for i = 2:1:length(in)
    
    new(i) = new_dble(i)*in(i - 1);
    
end

figure
subplot(211)
plot(in,'b')
hold on
plot(out,'r')
legend('Original Signal','PEAK');
title('Signal with PEAK envelope');
grid on
hold off
subplot(212)
plot(out)
hold on
plot(new);
legend('PEAK','Original signal after');
grid on
hold off

%audiowrite('D:\DRC\DRC_project\DRC_project\merrychristmasdarlingMono_out.wav',new,Fs);
%sound(new,Fs);

