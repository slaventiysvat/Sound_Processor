function lsf = poly2lsf_my(a)
%POLY2LSF Prediction polynomial to line spectral frequencies.
%   LSF = POLY2LSF(A) converts the prediction polynomial specified by A,
%   into the corresponding line spectral frequencies, LSF.
%
%   POLY2LSF normalizes the prediction polynomial by A(1).

a_temp = a;

a_temp  = a_temp(:);

% Normalize the polynomial if a(1) is not unity

if a_temp(1) ~= 1.0
    a_temp = a_temp./a_temp(1);
end

% Form the sum and difference filters

p  = length(a_temp) - 1;  % The leading one in the polynomial is not used
a1 = [a_temp;0];
a2 = a1(end:-1:1);
P1 = a1-a2;        % Difference filter
Q1 = a1+a2;        % Sum Filter

% If order is even, remove the known root at z = 1 for P1 and z = -1 for Q1
% If odd, remove both the roots from P1

if mod(p,2)   %Odd order
    P = deconv(P1,[1 0 -1]);
    Q = Q1;
else          %Even order
    P = deconv(P1,[1 -1]);
    Q = deconv(Q1,[1  1]);
end

rP  = roots(P);
rQ  = roots(Q);
 
% considering complex conjugate roots along with zeros for finding aP and
% aQ 
aP  = angle(rP(:,[1 1]));
aQ  = angle(rQ(:,[1 1]));

lsf_temp = sort([aP(aP >= 0);aQ(aQ >= 0)]); % considering positive angles
lsf =lsf_temp(1:2:end);
end