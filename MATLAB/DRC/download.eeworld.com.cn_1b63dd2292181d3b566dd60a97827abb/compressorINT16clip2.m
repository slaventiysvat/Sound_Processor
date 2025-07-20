function [y,G_out] = compressorINT16clip2(x,Fd,G_in,M)
%compressor with the same fixed-point algorithm with CCS
% y = compressorINT16clip2(x,Fd,G_in,M)
%x      -- the input signal array
%Fd     -- the full deviated amplitude
%G_in   -- the initial Gain
%M      -- update the compressor gain every M samples
%
%y      -- the compressed signal array
%G_out  -- the output Gain
%
%Companding Specificationn
%      Compression Ratio 2:1
%      Crossover Point   60% of full audio deviation
%      Attack Time       3.3 ms
%      Recovery Time     14.85 ms
%
%
INT16_SCALE = 32768;

persistent average counter mean old_x_array ac1 ac1_1 ac2 ac2_1;

%the input array x is averaged by the following formula
%   average = ac*average + (1-ac)*x;

%the mean stores the mean of the previous 40 input samples
%                  i
%   mean =(1/40)* SUM abs(x(k))
%               k=(i-39)

%initialization
if isempty(ac1)
    ac1 = float2Fract(0.98875);
    ac1_1 = 2^31 - fract2LongFract(ac1); %float2LongFract((1-0.98875));
    ac2 = float2Fract(0.9999);
    ac2_1 = 2^31 - fract2LongFract(ac2); %float2LongFract((1-0.9994));
    
    ac_diff = float2Fract(0.9995);
    ac_diff_1 = 2^31 - fract2LongFract(ac_diff);
    
    counter = 0;
    average = 0;
    average2 = 0;
    mean = 0;
    old_x_array = zeros(1,40);
end


ac = ac1;
ac_1 = ac1_1;
Uc = fractMult(float2Fract(0.6366),Fd);
sqrtUc = float2Fract(sqrt(fract2Float(Uc)));

%for test only
global mean_g average_g diff_g avg_dif_g G_in_g flag_g avg_dif_float_g average2_g;

avg_dif = 0;
avg_dif_float = 0;                              %test only  
OneForty = float2Fract(1/40);
temp1 = 0;
temp2 = 0;
scale_flag = 0;

for i = 1:length(x)
    abstract = abs(x(i));
    
  
    temp1 = longLongFractMult(temp1,ac) + longLongFractMult(ac_1,abstract);       %averager. divide 2^15 for fractional multiplication.
    average = longFract2Fract(temp1);
    
    
    %average the abstract with the primary coefficient
    temp2 = longLongFractMult(temp2,ac1) + longLongFractMult(ac1_1,abstract); 
    average2 = longFract2Fract(temp2); 
    average2_g(i) = average2;
    
    mean = mean - fractMult(OneForty,old_x_array(1)) + fractMult(OneForty,abstract);    %update mean
    
    mean_g(i)= mean;                            %test only
    
    if counter == 0                             %update gain every M times     
        if average == 0
            average = float2Fract(2^-15);                % 2^-15
        end
        sqrtAvg = float2Fract(sqrt(fract2Float(average)));
        G_in = sqrtUc/sqrtAvg;
        G_in_floor = floor(G_in);
        G_in_fract = float2Fract(G_in - floor(G_in));
        counter = M;
    end
    
    G_in_g(i) = G_in;
    
    old_x_array(1:39) = old_x_array(2:40);
    old_x_array(40) = abs(x(i));
    
    difference = abs(mean-average);
    
    diff_g(i) = difference;                     %test only
    average_g(i) = average;                     %test only
    
    if difference > fractMult(mean,float2Fract(0.1))        %0.1*mean
        avg_dif = fract2LongFract(difference);        %scale up 6 bits to prevent fix-point effect in calculating average difference
        avg_dif_float = difference;              %test only
        ac = ac1;
        ac_1 = ac1_1;
        
        flag_g(i) = 0;                          %test only
    else
        avg_dif = longLongFractMult(ac_diff_1,difference) + longLongFractMult(avg_dif,ac_diff);
        avg_dif_float = difference + 0.9995*(avg_dif_float-difference);   %test only
        
        if avg_dif < longLongFractMult(float2LongFract(0.04),mean)
            ac = ac2;
            ac_1 = ac2_1;
            flag_g(i) = 1000;                    %test only
        else
            ac = ac1;
            ac_1 = ac1_1;
            flag_g(i) = 0;                       %test only
        end
    end
       
    avg_dif_g(i) = avg_dif;                     %test only
    avg_dif_float_g(i) = avg_dif_float;         %test only
    
    counter = counter - 1;
    
    temp = fractMultInt(x(i),G_in_floor) + fractMult(G_in_fract,x(i));
    if temp >= INT16_SCALE - 1
        temp = INT16_SCALE - 1;
    end
    if temp < -INT16_SCALE
        temp = -INT16_SCALE;
    end
    y(i) = temp;
end

G_out = G_in;                               %store the current gain