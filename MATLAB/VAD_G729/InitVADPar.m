function VADPar = InitVADPar(Fs,Nw,Nsh,Ni)

% initialize constant parameters
VADPar.Init_Noise = 45;
VADPar.M = 42;     % LP order
VADPar.NP = 42;    % autocorrelation order
VADPar.Fs = Fs;
VADPar.Frame_Size = Nsh;
VADPar.N0 = 16;    % number of frames for long-term min energy calculation
VADPar.Ni = Ni;    % number of frames for initialization of running averages (32)
VADPar.INIT_COUNT = 20;
VADPar.In_Noise_threshould = zeros(1,Ni);
% HPFilt is a HPF that is used to preprocess the signal applied to the VAD.
% 140 Hz cutoff, unity gain near 200 Hz, falling to 0.971 at high freq.
VADPar.HPFilt.b = [0.895730417643409,-5.37438250586045,13.4359562646511,...
    -17.9146083528682,13.4359562646511,-5.37438250586045,0.895730417643409];
VADPar.HPFilt.a = [ 1,-5.77981206364905,13.9231708465036,-17.8928993556478...
    12.9378695523634,-4.99066192992268,0.802332981091635 ];
% VADPar.HPFilt.b = [ 0.92727435, -1.8544941,  0.92727435 ];
% VADPar.HPFilt.a = [ 1,          -1.9059465,  0.91140240 ];
VADPar.HPFilt.Mem = [];

VADPar.N = Nw;      % window size
VADPar.LA = 15;     % Look-ahead
VADPar.NF = Nsh;    % Frame size

LWmem = VADPar.N - VADPar.NF;
VADPar.Wmem = zeros(LWmem, 1);

LA = VADPar.LA;
LB = VADPar.N - VADPar.LA;
VADPar.Window = [0.54 - 0.46*cos(2*pi*(0:LB-1)'/(2*LB-1));
                 cos(2*pi*(0:LA-1)'/(4*LA-1))];

% LP analysis, lag window applied to autocorrelation coefficients
% Fs = 8000;
BWExp = 60;         % 60 Hz bandwidth expansion, Gaussian window
w0 = 2 * pi * BWExp / Fs;
NP = VADPar.NP;
Wn = 1.0001;        % White noise compensation (diagonal loading)
VADPar.LagWindow = [Wn; exp(-0.5 * (w0 * (1:NP)').^2)] / Wn;

% Correlation for a lowpass filter (3 dB point on the power spectrum is
% at about 2 kHz)
VADPar.LBF_CORR = ...
    [ 0.24017939691329, 0.21398822343783, 0.14767692339633, ...
      0.07018811903116, 0.00980856433051,-0.02015934721195, ...
     -0.02388269958005,-0.01480076155002,-0.00503292155509, ...
      0.00012141366508, 0.00119354245231, 0.00065908718613, ...
      0.00015015782285]';
% fvtool(VADPar.LBF_CORR,'Fs',8000)
% Correlation for a lowpass filter (on the power spectrum is
% at about 3 kHz)
% VADPar.LBF_CORR = ...
% [-0.00218390731980220,	-0.000556485648462058,	0.0124903011598045,...
%   0.00224187806695614,	-0.0444414795239154,...
%  -0.00421593986568836,	0.189172861671438,...	
%   0.315198646869439,	0.189172861671438,	-0.00421593986568836,...	
%  -0.0444414795239154,	0.00224187806695614,...
%   0.0124903011598045,	-0.000556485648462058,	-0.00218390731980220];


% initialize variable parameters
VADPar.FrmCount = 0;
VADPar.FrmEn = Inf * ones(1,VADPar.N0);
VADPar.MeanLSF = zeros(VADPar.M, 1);
VADPar.MeanSE = 0;
VADPar.MeanSLE = 0;
VADPar.MeanE = 0;
VADPar.MeanSZC = 0;
VADPar.count_sil = 0;
VADPar.count_inert = 0;     % modified for AppendixII
VADPar.count_update = 0;
VADPar.count_ext = 0;
VADPar.less_count = 0;
VADPar.flag = 1;

VADPar.PrevMarkers = [1, 1];
VADPar.PrevEnergy = 0;

VADPar.Prev_MinE = Inf;
VADPar.Next_MinE = Inf;
VADPar.MinE_buffer = Inf * ones(1, VADPar.N0/8);

end