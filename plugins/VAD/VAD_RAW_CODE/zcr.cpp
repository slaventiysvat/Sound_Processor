#include "zcr.hpp"

// Calculate zero-crossing rate using Eigen operations.
double zcr(const Eigen::VectorXd& x) {
    const Eigen::Index n = x.size();
    if (n <= 1) {
        return 0.0;
    }
    // xp holds products of consecutive samples
    Eigen::VectorXd xp = x.head(n - 1).cwiseProduct(x.tail(n - 1));
    // Count sign changes where product is negative
    double crossings = (xp.array() < 0).cast<double>().sum();
    return crossings / static_cast<double>(n - 1);
}