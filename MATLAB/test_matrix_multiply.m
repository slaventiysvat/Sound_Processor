clear
clc
A = ones(5,10).*2.0;
B = ones(10,5).*2.0;
rez1 = A * B;


C1 = eye(5,5);
C1 = C1';

n = 1:1:10;
m = 10;
p = 10;
y = n(m:-1:m-p+1);

% b = 1;
% for k = m:-1:1
%     y(b) = n(k);
%     b = b + 1;
% end