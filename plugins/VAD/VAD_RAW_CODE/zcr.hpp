#pragma once

#include <Eigen/Dense>

// Computes the zero-crossing rate of a signal vector x.
// The zero-crossing rate is the fraction of consecutive sample pairs
// that change sign.
double zcr(const Eigen::VectorXd& x);