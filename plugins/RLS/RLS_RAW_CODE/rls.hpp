#pragma once
#include <Eigen/Dense>

struct RLSState {
    int p;                // filter order
    double lambda;        // forgetting factor
    double lam_inv;       // inverse forgetting factor
    double delta;         // initial diagonal value
    Eigen::VectorXd w;    // filter coefficients
    Eigen::MatrixXd P;    // inverse correlation matrix
    Eigen::VectorXd buff_n; // noise buffer
    Eigen::VectorXd buff_x; // signal buffer
};

RLSState rls_init(int order, double fr_factor, double delta_param);

Eigen::VectorXd RLSFilterIt(const Eigen::VectorXd &n,
                            const Eigen::VectorXd &x,
                            RLSState &st,
                            int vad_dec);