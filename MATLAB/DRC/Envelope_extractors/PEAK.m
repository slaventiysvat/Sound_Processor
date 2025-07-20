function out = PEAK(in,attack_time,release_time,Fs,previous_y)

    AT = (-2.2 *(1.0 / Fs)) / (attack_time / 1000.0);
    RT = (-2.2 *(1.0 / Fs)) / (release_time / 1000.0);
    AT = 1.0 - exp(AT);
    RT = 1.0 - exp(RT);
    
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

end