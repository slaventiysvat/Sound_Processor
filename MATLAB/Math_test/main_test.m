close all
clc
clear all
A = [1 2 3 4 5 6;           %matrix 6x6  6x3
     7 8 9 10 11 12;
     13 14 15 16 17 18; 
     19 20 21 22 23 24;
     25 26 27 28 29 30;
     31 32 33 34 35 36];
 
 B = [1 2 3 4 5 6;
     7 8 9 10 11 12;
     19 20 21 22 23 24;];   %matrix 3x6
 B = B';
 
 B_1 = [1 2 3 5;
        7 8 10 25;
        19 20 21 48;
        19 20 21 66;
        19 20 21 24;
        19 20 21 30];   %matrix 6x4
 disp(A);
 
 A_trz = A';%transponze matrix
 A_rez = matrix_transponze(A);
 disp("My transpose loop")
 disp(A_rez);
 disp("Matlab transpose loop")
 disp(A_trz);
 
 A_trz_2 = A_trz';%transponze matrix
 disp( A_trz_2);
 
 C = A * B;
 C1 = A * B_1;

 K1 = ones(4,6);
 K2 = ones(6,5);
 
 K3 = K2 * K1;
 
 disp(K3);
 
 function A_rez = matrix_transponze(A)
 
 %transponze loop
 
 [m,n] = size(A);
 A_rez = zeros(n,m);
 
 for i = 1:1:n
     for j = 1:1:m
         A_rez(i,j) = A(j,i);
     end
 end
 end
 
 