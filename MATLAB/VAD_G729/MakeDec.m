function dec = MakeDec(dSLE, dSE, SD, dSZC)

a = [0.00175, -0.004545455, -25, 20, 0, ...
     8800, 0, 25, -29.09091, 0, ...
     14000, 0.928571, -1.5, 0.714285];

b = [0.00085, 0.001159091, -5, -6, -4.7, ...
     -12.2, 0.0009, -7.0, -4.8182, -5.3, ...
     -15.5, 1.14285, -9, -2.1428571];

dec = 0;

% SD vs dSZC
if SD > a(1)*dSZC+b(1)
    dec = 1;
    return;
end

if SD > a(2)*dSZC+b(2)
    dec = 1;
    return;
end

% dSE vs dSZC
if dSE < a(3)*dSZC+b(3)
    dec = 1;
    return;
end

if dSE < a(4)*dSZC+b(4)
    dec = 1;
    return;
end

if dSE < b(5)
    dec = 1;
    return;
end
    
% dSE vs SD       
if dSE < a(6)*SD+b(6)
    dec = 1;
    return;
end

if SD > b(7)
    dec = 1;
    return;
end

% dSLE vs dSZC
if dSLE < a(8)*dSZC+b(8)
    dec = 1;
    return;
end

if dSLE < a(9)*dSZC+b(9)
    dec = 1;
    return;
end

if dSLE < b(10)
    dec = 1;
    return;
end

% dSLE vs SD
if dSLE < a(11)*SD+b(11)
    dec = 1;
    return;
end

% dSLE vs dSE
if dSLE > a(12)*dSE+b(12)
    dec = 1;
    return
end

if dSLE < a(13)*dSE+b(13)
    dec = 1;
    return;
end

if dSLE < a(14)*dSE+b(14)
    dec = 1;
    return;
end

end

