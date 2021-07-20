function coefs = filter_coefs_manager(f_1,f_2,Fs,Taps)
frequency = Fs/2;
start_freq = f_1 / frequency;
end_freq = f_2 / frequency;
coefs = fir1(Taps-1,[start_freq end_freq])';
% fvtool(Pz,'Fs',Fs)
end