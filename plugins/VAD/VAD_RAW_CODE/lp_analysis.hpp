#pragma once

#include <Eigen/Dense>

namespace vad {

struct VADParams; // forward declaration

struct LPResult {
    Eigen::VectorXd r;   ///< Autocorrelation values
    Eigen::VectorXd LSF; ///< Normalized line spectral frequencies
    double rc2;          ///< Second reflection coefficient
};

LPResult vadLpAnalysis(const Eigen::VectorXd& x, const VADParams& p);

} 