function [Ivd, VADPar, v_flag] = VAD (x_new, VADPar)
% The Matlab routine implements the Voice Activity Detector (VAD) for
% the ITU-T G.729 coder. The VAD is specified in G.729B (annex B to
% G.729) to accompany G.729A the low complexity version of the G.729 coder.
% There is a modification to the VAD given in Appendix II (G.729II).
%
% The reference code for G.729A, G.729B, and G.729II uses fixed point
% arithmetic. However, G.729C+ includes reference code in floating point
% for both the coder and the VAD. This Matlab routine in double precision
% floating point borrows the relevant parts from the Annex C+ floating
% point code, but retains the decision logic of Appendix II. A switch is
% available to disable the Appendix II modifications.

% The VAD uses the preprocessed speech (highpass filtered) and the linear
% predictive parameters from the coder. The Matlab code here is standalone
% and so includes the preprocessing and the LP analysis.

% Tests on this VAD show a match to the G.729C+ VAD decisions (with the
% Appendix II option turned off).

% P. Kabal 2008-04-03

% Ivd - VAD flag, 0 no speech, 1 speech
% VADPar - Updated parameter structure
% v_flag - one during hangover (only for VAD_APPENDIX_II = 0)

VAD_APPENDIX_II = 1;

% Constants
N = VADPar.N;    % window size
N0 = VADPar.N0;  % number of frames used for long-term minimum energy calculation
Ni = VADPar.Ni;  % number of frames used for initialization of running averages
INIT_COUNT = VADPar.INIT_COUNT;
NOISE = 0;
VOICE = 1;
v_flag = 0;
Init_Noise = VADPar.Init_Noise;
VADPar.FrmCount = VADPar.FrmCount + 1;
frm_count = VADPar.FrmCount;

% Filter new data (HP filter)

[x_new_hp, VADPar.HPFilt.Mem] = filter(VADPar.HPFilt.b, VADPar.HPFilt.a, ...
                                       32768 * x_new, VADPar.HPFilt.Mem);
                                                              
% Append new filtered data to filter memory
xwin = [VADPar.Wmem; x_new_hp];

% LPC analysis
[r, LSF, rc2] = VADLPAnalysis(xwin, VADPar);

% Full band energy
Ef = 10*log10(r(1) / N);
r = r(1:length(VADPar.LBF_CORR));
% Low band energy
Elow = r(1) * VADPar.LBF_CORR(1) ...
       + 2 * sum(r(2:end) .* VADPar.LBF_CORR(2:end));
El = 10*log10(Elow / N);

% Compute SD % Spectral Distortion
SD = sum((LSF-VADPar.MeanLSF).^2);

% Normalized zero-crossing rate (in current frame)
ist = VADPar.N - VADPar.LA - VADPar.NF + 1;     % Current frame start
ifn = ist + VADPar.NF - 1;                      % Current frame end
ZC = zcr(xwin(ist:ifn+1));

% The next steps involve finding the minimum energy in the N0 frames.
% The original code in G.729 is very convoluted. The Matlab code below
% mimics the operation with a simpler structure.
% - To reduce computations, the minimum energy for blocks of 8 samples
%   is determined. These values are stored in a buffer of length N0/8.
%   The buffer is updated whenever the frame count is a multiple of 8.
%   Starting at the beginning, the minimum of the frames 1-8 is stored
%   into the buffer in frame 8, the minimum of the frames 9-16 is stored
%   into the buffer at frame 16, etc.
% - Prev_Min is the minimum of the values stored in the buffer, effectively
%   the minimum of N0 energy values.
% - Next_Min is the minimum used to determine the minimum of the next
%   8 samples.
% - MinE is min(Prev_Min, Next_Min).
% - Note that that for frame count equal to a multiple of 8, Next_Min is
%   updated and MinE is updated before updating the buffer. This means
%   that MinE is calculated over N0+8 values. MinE is effectively
%   calculated over a varying window length (N0+1 to N0+8). It is
%   nonincreasing while the window length increases.
% - The value of Min will not be used until frame N0.

% Long-term minimum energy
VADPar.Next_MinE = min(Ef, VADPar.Next_MinE);
MinE = min(VADPar.Prev_MinE, VADPar.Next_MinE);
if (mod(frm_count, 8) == 0)
  VADPar.MinE_buffer = [VADPar.MinE_buffer(2:end), VADPar.Next_MinE];
  VADPar.Prev_MinE = min(VADPar.MinE_buffer);
  VADPar.Next_MinE = Inf;
end

% Initialization of running averages
%Assumptinos - the first Ni frames is always Noise 
%======================
% if(frm_count <= Ni && frm_count > 1)
%     marker = NOISE;
%     VADPar.In_Noise_threshould(frm_count) = Ef;
%     VADPar.In_Noise_threshould(frm_count) = max(VADPar.In_Noise_threshould(1:frm_count));
% else
%     VADPar.In_Noise_threshould(1) = Ef;
% end


%====================
if (frm_count <= Ni)
  if (Ef <= Init_Noise)
    VADPar.less_count = VADPar.less_count + 1;
    marker = NOISE;
  else
    marker = NOISE;
    NEp = (frm_count - 1) - VADPar.less_count;
    NE = NEp + 1;
    VADPar.MeanE = (VADPar.MeanE * NEp + Ef) / NE;
    VADPar.MeanSZC = (VADPar.MeanSZC * NEp + ZC) / NE;
    VADPar.MeanLSF = (VADPar.MeanLSF * NEp + LSF) / NE;
  end

end
 
% 
% if (frm_count == Ni)
%     VADPar.Init_Noise = min(VADPar.In_Noise_threshould(1:frm_count));
% end

% if (frm_count > Ni && VADPar.PrevMarkers(1)== NOISE && VADPar.PrevMarkers(2)== NOISE)
%     VADPar.Init_Noise = max(VADPar.Init_Noise,Ef);
% end

if (frm_count >= Ni)
  if (frm_count == Ni)
    if (VAD_APPENDIX_II)
      if (VADPar.less_count >= Ni)    % modified for Appendix II
        VADPar.FrmCount = 0;
        frm_count = VADPar.FrmCount;
        VADPar.less_count = 0;
      end
    end
      VADPar.MeanSE = VADPar.MeanE - 10;%10
      VADPar.MeanSLE = VADPar.MeanE - 12;%12
%     VADPar.MeanSE = VADPar.MeanE - 1;%10
%     VADPar.MeanSLE = VADPar.MeanE - 2;%12
%      VADPar.MeanSE = VADPar.MeanE;%10
%      VADPar.MeanSLE = VADPar.MeanE;%12
  end

 dSE = VADPar.MeanSE - Ef;
 dSLE = VADPar.MeanSLE - El;
 dSZC = VADPar.MeanSZC - ZC;


  if (Ef < Init_Noise)
    marker = NOISE;
  else
    marker = MakeDec(dSLE, dSE, SD, dSZC);
  end

  if (VAD_APPENDIX_II)
    if (marker == VOICE)             % modified for Appendix II
      VADPar.count_inert = 0;
    end
    %Make desithion of voice signal soft
    if (marker == NOISE && VADPar.count_inert < 3)%was 6
      VADPar.count_inert = VADPar.count_inert + 1;
      marker = VOICE;
%         marker = NOISE;
    end
    
  else
    v_flag = 0;
  end
    
% Voice activity decision smoothing: Step 1
  if (VADPar.PrevMarkers(1) == VOICE && marker == NOISE ...
      && Ef > VADPar.MeanSE + 2 && Ef >  Init_Noise)
    marker = VOICE;
    if (~VAD_APPENDIX_II)
      v_flag = 1;
    end
  end
    
    % Voice activity decision smoothing: Step 2
  if (VADPar.flag == 1)
    if (VADPar.PrevMarkers(2) == VOICE ...
        && VADPar.PrevMarkers(1) == VOICE ...
        && marker == NOISE ...
        && abs(Ef - VADPar.PrevEnergy) <= 3)
      VADPar.count_ext = VADPar.count_ext + 1;
      marker = VOICE;
      if(~ VAD_APPENDIX_II)
        v_flag = 1;
      end
            
      if (VADPar.count_ext <= 10)%was 4 
        VADPar.flag = 1;
      else
        VADPar.flag = 0;
        VADPar.count_ext = 0;
      end
    end
  else
    VADPar.flag = 1;
  end

% For unvoiced case, count_sil is incremented
  if (marker == NOISE)
     VADPar.count_sil = VADPar.count_sil + 1;
  end
  
% Voice activity decision smoothing: Step 3    
  if (marker == VOICE && VADPar.count_sil >= 5 ... was 10
      && Ef - VADPar.PrevEnergy <= 3)
    marker = NOISE;
    VADPar.count_sil = 0;
    if (VAD_APPENDIX_II)
%        VADPar.count_inert = 6;  % modified for AppendixII
        VADPar.count_inert = 6;  % modified for AppendixII
    end
  end
    
  if (marker == VOICE)
    VADPar.count_sil = 0;
  end

% Voice activity decision smoothing: Step 4
  if (~VAD_APPENDIX_II)
    if (Ef < VADPar.MeanSE + 2 && VADPar.FrmCount > N0 ...
        && v_flag == 0 && rc2 < 0.6)
      marker = NOISE;
    end
  end

  if (VAD_APPENDIX_II)
    TestC = (Ef < VADPar.MeanSE + 3 && rc2 < 0.75);       % Appendix II
  else
    TestC = (Ef < VADPar.MeanSE + 3 && rc2 < 0.75 && SD < 0.002532959);
  end
  if (TestC)
    VADPar.count_update = VADPar.count_update + 1;
    % Modify update speed coefficients
    if (VADPar.count_update < INIT_COUNT)
      COEF = 0.75;
      COEFZC = 0.8;
      COEFSD = 0.6;
    elseif (VADPar.count_update < INIT_COUNT + 10)
      COEF = 0.95;
      COEFZC = 0.92;
      COEFSD = 0.65;
    elseif (VADPar.count_update < INIT_COUNT + 20)
      COEF = 0.97;
      COEFZC = 0.94;
      COEFSD = 0.70;
    elseif (VADPar.count_update < INIT_COUNT + 30)
      COEF = 0.99;
      COEFZC = 0.96;
      COEFSD = 0.75;
    elseif (VADPar.count_update < INIT_COUNT + 40)
      COEF = 0.995;
      COEFZC = 0.99;
      COEFSD = 0.75;
    else
      COEF = 0.995;
      COEFZC = 0.998;
      COEFSD = 0.75;
    end

% Update mean LSF, SE, SLE, SZC
    VADPar.MeanLSF = COEFSD * VADPar.MeanLSF + (1-COEFSD) * LSF;
    VADPar.MeanSE = COEF * VADPar.MeanSE + (1-COEF) * Ef;
    VADPar.MeanSLE = COEF * VADPar.MeanSLE + (1-COEF) * El;
    VADPar.MeanSZC = COEFZC * VADPar.MeanSZC + (1-COEFZC) * ZC;
  end

  if (frm_count > N0 && ...
        (VADPar.MeanSE < MinE && SD < 0.002532959) ...
          || VADPar.MeanSE > MinE + 10 )
    VADPar.MeanSE = MinE;
    VADPar.count_update = 0;
  end
end

VADPar.PrevEnergy = Ef;
VADPar.PrevMarkers = [marker, VADPar.PrevMarkers(1)];

ist = VADPar.NF + 1;
VADPar.Wmem = xwin(ist:end);
Ivd = marker;
end