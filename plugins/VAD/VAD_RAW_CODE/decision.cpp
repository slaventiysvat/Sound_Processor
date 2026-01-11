#include "decision.hpp"

int makeDec(double dSLE, double dSE, double SD, double dSZC) {
    static const double a[] = {
        0.00175, -0.004545455, -25, 20, 0,
        8800, 0, 25, -29.09091, 0,
        14000, 0.928571, -1.5, 0.714285
    };

    static const double b[] = {
        0.00085, 0.001159091, -5, -6, -4.7,
        -12.2, 0.0009, -7.0, -4.8182, -5.3,
        -15.5, 1.14285, -9, -2.1428571
    };

    if (SD > a[0] * dSZC + b[0]) return 1;
    if (SD > a[1] * dSZC + b[1]) return 1;
    if (dSE < a[2] * dSZC + b[2]) return 1;
    if (dSE < a[3] * dSZC + b[3]) return 1;
    if (dSE < b[4]) return 1;
    if (dSE < a[5] * SD + b[5]) return 1;
    if (SD > b[6]) return 1;
    if (dSLE < a[7] * dSZC + b[7]) return 1;
    if (dSLE < a[8] * dSZC + b[8]) return 1;
    if (dSLE < b[9]) return 1;
    if (dSLE < a[10] * SD + b[10]) return 1;
    if (dSLE > a[11] * dSE + b[11]) return 1;
    if (dSLE < a[12] * dSE + b[12]) return 1;
    if (dSLE < a[13] * dSE + b[13]) return 1;

    return 0;
}