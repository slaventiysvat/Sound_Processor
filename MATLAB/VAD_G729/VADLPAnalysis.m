function [r, LSF, rc2] = VADLPAnalysis(x, VADPar)
M = VADPar.M;    % LP order
NP = VADPar.NP;  % autocorrelation order
% Apply window to input frame
xw = VADPar.Window .* x;
% Compute autocorrelation
r = acorr(xw, NP+1) .* VADPar.LagWindow;
% Compute normalized LSF
% A = ac2poly(r(1:M+1));
N = length(r(1:M+1)) - 1;
[pred_coef,pred_var,ref_coef]= levinson_dublin(r(1:M+1),N); 
A = [1,pred_coef];
% figure
% plot(A)
% hold on
% plot(pred_coef)
LSF = poly2lsf(A) / (2 * pi);    % normalized to 0 to 0.5
% Reflection coefficients
rc = ac2rc(r(1:3));
rc2 = rc(2);
end
