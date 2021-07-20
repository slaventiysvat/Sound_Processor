function rxx = acorr (x, Nt)

Nx = length (x);
N = Nt;
if (Nt > Nx)
  N = Nx;
end

rxx = zeros(Nt, 1);
for i = 0:1:N-1
  Nv = Nx - i;
  rxx(i+1) = x(1:Nv)' * x(i+1:i+Nv);
end

end