% ----- ----- ----- -----
function [zc] = zcr (x)
% Calculate normalized (per sample) zero-crossing rate
% Input is the frame plus the first sample of the next
% frame.

M = length(x) - 1;

x1 = x(1:end-1);
x2 = x(2:end);

xp = x1 .* x2;
I = (xp < 0);

%sign1 = sign(x);
%sign2 = sign([mem; x(1:M-1)]);
%
%zc = 1/(2*M)*sum(abs(sign1-sign2));

zc = sum(I) / M;
end