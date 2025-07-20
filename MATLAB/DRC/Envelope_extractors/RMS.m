function out = RMS(in,N,previous_y)

%-------------------------------RMS detector------------------------------------

      %RMS_detector
      out = zeros(1,length(in));

      out(1) = (previous_y * ((N - 1.0) / N)) + ((in(1)^2) * (1.0 / N));

      for i = 2:1:length(in)
       
        out(i) = (out(i - 1) * ((N - 1.0) / N)) + ((in(i)^2) * (1.0 / N));

      end

      for i = 1:1:length(in)

          out(i) = sqrt(out(i));

      end
end