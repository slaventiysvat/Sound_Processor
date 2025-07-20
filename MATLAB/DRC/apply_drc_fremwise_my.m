function output = apply_drc_framewise(signal, Fs, frame_size, hop_size, params, use_rms)
% APPLY_DRC_FRAMEWISE - Frame-based Dynamic Range Compressor with HPF and overlap-add
%
%   output = apply_drc_framewise(signal, Fs, frame_size, hop_size, params, use_rms)
%
% Inputs:
%   signal      - Input mono audio vector
%   Fs          - Sampling rate (8000–44100 Hz)
%   frame_size  - Frame size (256–2048 samples)
%   hop_size    - Hop size between frames (e.g., 50% overlap = frame_size/2)
%   params      - Struct with DRC parameters (+ .hpf_cutoff in Hz)
%   use_rms     - true = RMS detector, false = Peak detector
%
% Output:
%   output      - DRC-processed signal (same length as input)

signal = signal(:);  % ensure column vector
signal = signal / max(abs(signal)); %normalizing
N = length(signal);
window = hann(frame_size);
output = zeros(N + frame_size, 1);
norm_window = zeros(N + frame_size, 1);

% --- High-pass filter design ---
if isfield(params, 'hpf_cutoff') && params.hpf_cutoff > 0
    cutoff = min(params.hpf_cutoff, Fs / 2 - 100); % avoid Nyquist
    [bh, ah] = butter(6, cutoff / (Fs / 2), 'high');
    apply_hpf = true;
else
    apply_hpf = false;
end

for start = 1:hop_size:(N - frame_size + 1)
    idx = start:(start + frame_size - 1);
    frame = signal(idx) .* window;

    % --- Optional high-pass filter to reduce rumble ---
    if apply_hpf
        frame = filter(bh, ah, frame);
    end

    % --- Envelope detection ---
    if use_rms
        alpha = 1.0 / round(Fs * params.release_time / 1000);
        power = 0;
        envelope = zeros(frame_size, 1);
        for i = 1:frame_size
            power = (1 - alpha) * power + alpha * frame(i)^2;
            envelope(i) = sqrt(power);
        end
    else
        AT = 1 - exp(-2.2 / (Fs * params.attack_time / 1000));
        RT = 1 - exp(-2.2 / (Fs * params.release_time / 1000));
        envelope = zeros(frame_size, 1);
        prev = 0;
        for i = 1:frame_size
            a = abs(frame(i));
            if a > prev
                envelope(i) = (1 - AT) * prev + AT * a;
            else
                envelope(i) = (1 - RT) * prev + RT * prev;
            end
            prev = envelope(i);
        end
    end

    % --- Gain computation ---
    gain = ones(frame_size, 1);
    for i = 1:frame_size
        a = envelope(i);
        if a < params.threshold_gate
            gain(i) = a / params.threshold_gate * params.ng_slope;
        elseif a < params.threshold_expander
            gain(i) = a / params.threshold_expander * params.ex_slope;
        elseif a < params.threshold_compressor
            gain(i) = params.flat_gain;
        elseif a < params.threshold_limiter
            gain(i) = 1 - (a - params.threshold_compressor) * params.cm_ratio;
        else
            gain(i) = params.limit_level / a;
        end
    end

    % --- Apply gain ---
    processed = frame .* gain;

    % --- Overlap-add reconstruction ---
    output(idx) = output(idx) + processed .* window;
    norm_window(idx) = norm_window(idx) + window.^2;
end

% --- Normalize overlap-add sum ---
norm_window(norm_window == 0) = 1e-6;
output = output(1:N) ./ norm_window(1:N);

end


% function output = apply_drc_fremwise_my(signal, Fs, frame_size, hop_size, params, use_rms)
% % APPLY_DRC_FRAMEWISE - Frame-based Dynamic Range Compressor with overlap-add
% %
% %   output = apply_drc_framewise(signal, Fs, frame_size, hop_size, params, use_rms)
% %
% % Inputs:
% %   signal      - Input mono audio vector
% %   Fs          - Sampling rate (8000–44100 Hz)
% %   frame_size  - Frame size (256–2048 samples)
% %   hop_size    - Hop size between frames (e.g., 50% overlap → hop = frame_size/2)
% %   params      - Struct with DRC parameters (same as in apply_drc)
% %   use_rms     - true = RMS detector, false = Peak detector
% %
% % Output:
% %   output      - Compressed signal (same length as input)
% 
% signal = signal(:);
% N = length(signal);
% window = hann(frame_size);
% output = zeros(N + frame_size, 1);
% norm_window = zeros(N + frame_size, 1);
% 
% for start = 1:hop_size:(N - frame_size + 1)
%     idx = start:(start + frame_size - 1);
%     frame = signal(idx) .* window;
% 
%     % Envelope detection
%     if use_rms
%         % RMS detector
%         alpha = 1.0 / round(Fs * params.release_time / 1000);
%         power = 0;
%         envelope = zeros(frame_size, 1);
%         for i = 1:frame_size
%             power = (1 - alpha) * power + alpha * frame(i)^2;
%             envelope(i) = sqrt(power);
%         end
%     else
%         % Peak detector
%         AT = 1 - exp(-2.2 / (Fs * params.attack_time / 1000));
%         RT = 1 - exp(-2.2 / (Fs * params.release_time / 1000));
%         envelope = zeros(frame_size, 1);
%         prev = 0;
%         for i = 1:frame_size
%             a = abs(frame(i));
%             if a > prev
%                 envelope(i) = (1 - AT) * prev + AT * a;
%             else
%                 envelope(i) = (1 - RT) * prev + RT * prev;
%             end
%             prev = envelope(i);
%         end
%     end
% 
%     % Gain computation
%     gain = ones(frame_size, 1);
%     for i = 1:frame_size
%         a = envelope(i);
%         if a < params.threshold_gate
%             gain(i) = a / params.threshold_gate * params.ng_slope;
%         elseif a < params.threshold_expander
%             gain(i) = a / params.threshold_expander * params.ex_slope;
%         elseif a < params.threshold_compressor
%             gain(i) = params.flat_gain;
%         elseif a < params.threshold_limiter
%             gain(i) = 1 - (a - params.threshold_compressor) * params.cm_ratio;
%         else
%             gain(i) = params.limit_level / a;
%         end
%     end
% 
%     % Apply gain
%     processed = frame .* gain;
% 
%     % Overlap-add
%     output(idx) = output(idx) + processed .* window;
%     norm_window(idx) = norm_window(idx) + window.^2;
% end
% 
% % Normalize overlap-add
% norm_window(norm_window == 0) = 1e-6;
% output = output(1:N) ./ norm_window(1:N);
% 
% end
