#include <gtest/gtest.h>
#include <Eigen/Dense>
#include "rls/rls.hpp"

TEST(RLSTest, ReducesNoise) {
    const int N = 1024;
    Eigen::VectorXd noise = Eigen::VectorXd::Random(N) * 0.1;
    Eigen::VectorXd signal(N);
    for (int i = 0; i < N; ++i) {
        signal(i) = std::sin(2.0 * M_PI * 440.0 * i / 16000.0);
    }
    Eigen::VectorXd x = signal + noise;

    RLSState st = rls_init(25, 1.0, 0.001);
    Eigen::VectorXd e = RLSFilterIt(noise, x, st, 0);

    double mse_before = (x - signal).squaredNorm() / N;
    double mse_after = (e - signal).squaredNorm() / N;

    ASSERT_LT(mse_after, mse_before);
}