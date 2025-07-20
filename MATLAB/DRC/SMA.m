 clear 
 clc
 [in,Fs] = audioread('..\..\DRC_project\test_sounds\meteo.wav');
 x = in;
 out = zeros(1,length(in));
 k = 0;
 sum = 0;
 N = 15;
 N_cof = 1 / N;
 for i = N:1:length(in);
   
  j = i;
  
      while k!=N-1

       sum = sum + x(j);
       k = k + 1;
       j = j - 1;
       
      end
      
  out(i) = sum * N_cof;
  sum = out(i);   
  k = 0;
  end
  
  
  figure 
  plot(out,'r');
  
  sound(out,Fs);