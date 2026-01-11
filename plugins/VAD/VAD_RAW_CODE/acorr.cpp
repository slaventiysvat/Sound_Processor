#include <algorithm>
#include "acorr.hpp"

namespace vad {

Eigen::VectorXd acorr(const Eigen::VectorXd& x, int Nt) {
    const int N = static_cast<int>(x.size());
    const int M = std::min(Nt, N);
    Eigen::VectorXd r(M);
    for (int i = 0; i < M; ++i) {
        r[i] = x.head(N - i).dot(x.segment(i, N - i));
    }
    return r;
}

} 