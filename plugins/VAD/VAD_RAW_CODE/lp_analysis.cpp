#include "vad/lp_analysis.hpp"

#include <algorithm>
#include <vector>
#include <complex>
#include <unsupported/Eigen/PolynomialSolver>

#include "vad/acorr.hpp"
#include "vad/vad_params.hpp"

namespace vad {
namespace {

// Polynomial deconvolution (division) assuming coefficients in ascending order
Eigen::VectorXd deconv(const Eigen::VectorXd& num, const Eigen::VectorXd& den) {
    const int n = static_cast<int>(num.size());
    const int m = static_cast<int>(den.size());
    const int k = n - m + 1;
    Eigen::VectorXd quotient = Eigen::VectorXd::Zero(k);
    Eigen::VectorXd remainder = num;
    for (int i = k - 1; i >= 0; --i) {
        double coef = remainder(m + i - 1) / den(m - 1);
        quotient(i) = coef;
        for (int j = 0; j < m; ++j) {
            remainder(i + j) -= coef * den(j);
        }
    }
    return quotient;
}

// Convert LPC polynomial to line spectral frequencies
Eigen::VectorXd poly2lsf(const Eigen::VectorXd& a) {
    Eigen::VectorXd a_norm = a;
    if (a_norm.size() == 0) {
        return Eigen::VectorXd();
    }
    // Normalize
    a_norm /= a_norm(0);
    const int p = static_cast<int>(a_norm.size()) - 1;

    Eigen::VectorXd a1(p + 2);
    a1.head(p + 1) = a_norm;
    a1(p + 1) = 0.0;
    Eigen::VectorXd a2 = a1.reverse();

    Eigen::VectorXd P1 = a1 - a2; // Difference filter
    Eigen::VectorXd Q1 = a1 + a2; // Sum filter

    Eigen::VectorXd P, Q;
    if (p % 2 == 1) { // Odd order
        Eigen::VectorXd denom(3);
        denom << 1.0, 0.0, -1.0;
        P = deconv(P1, denom);
        Q = Q1;
    } else { // Even order
        Eigen::VectorXd denomP(2), denomQ(2);
        denomP << 1.0, -1.0;
        denomQ << 1.0, 1.0;
        P = deconv(P1, denomP);
        Q = deconv(Q1, denomQ);
    }

    auto rootsToAngles = [](const Eigen::VectorXd& coeff) {
        Eigen::PolynomialSolver<double, Eigen::Dynamic> solver;
        solver.compute(coeff);
        Eigen::VectorXcd roots = solver.roots();
        std::vector<double> angles;
        angles.reserve(roots.size());
        for (int i = 0; i < roots.size(); ++i) {
            if (roots(i).imag() >= 0) {
                double angle = std::atan2(roots(i).imag(), roots(i).real());
                if (angle >= 0) {
                    angles.push_back(angle);
                }
            }
        }
        std::sort(angles.begin(), angles.end());
        Eigen::VectorXd out(angles.size());
        for (int i = 0; i < static_cast<int>(angles.size()); ++i) {
            out(i) = angles[i];
        }
        return out;
    };

    Eigen::VectorXd aP = rootsToAngles(P);
    Eigen::VectorXd aQ = rootsToAngles(Q);

    std::vector<double> allAngles;
    allAngles.reserve(aP.size() + aQ.size());
    for (int i = 0; i < aP.size(); ++i) allAngles.push_back(aP(i));
    for (int i = 0; i < aQ.size(); ++i) allAngles.push_back(aQ(i));
    std::sort(allAngles.begin(), allAngles.end());
    Eigen::VectorXd lsf(allAngles.size() / 2);
    for (int i = 0, j = 0; j < lsf.size(); i += 2, ++j) {
        lsf(j) = allAngles[i];
    }
    return lsf;
}

// Levinson-Durbin recursion to obtain LPC and reflection coefficients
struct LevinsonResult {
    Eigen::VectorXd a;  // predictor coefficients
    Eigen::VectorXd k;  // reflection coefficients
};

LevinsonResult levinsonDurbin(const Eigen::VectorXd& r) {
    const int p = static_cast<int>(r.size()) - 1;
    LevinsonResult res;
    res.a = Eigen::VectorXd::Zero(p);
    res.k = Eigen::VectorXd::Zero(p);
    double E = r(0);

    for (int i = 0; i < p; ++i) {
        double acc = r(i + 1);
        for (int j = 0; j < i; ++j) {
            acc += res.a(j) * r(i - j);
        }
        double ki = -acc / E;
        res.k(i) = ki;

        Eigen::VectorXd a_new = res.a.head(i);
        for (int j = 0; j < i; ++j) {
            a_new(j) += ki * res.a(i - 1 - j);
        }
        res.a.head(i) = a_new;
        res.a(i) = ki;
        E *= 1.0 - ki * ki;
    }
    return res;
}

// Convert autocorrelation to reflection coefficients
Eigen::VectorXd ac2rc(const Eigen::VectorXd& r) {
    return levinsonDurbin(r).k;
}

} // namespace

LPResult vadLpAnalysis(const Eigen::VectorXd& x, const VADParams& p) {
    Eigen::VectorXd xw = x.cwiseProduct(p.Window);
    Eigen::VectorXd r = acorr(xw, p.NP + 1).cwiseProduct(p.LagWindow);

    // LPC analysis using Levinson-Durbin
    Eigen::VectorXd rM = r.head(p.M + 1);
    LevinsonResult lev = levinsonDurbin(rM);
    Eigen::VectorXd A(1 + lev.a.size());
    A(0) = 1.0;
    A.tail(lev.a.size()) = lev.a;
    Eigen::VectorXd LSF = poly2lsf(A) / (2.0 * M_PI);

    Eigen::VectorXd rc = ac2rc(r.head(3));
    double rc2 = rc.size() > 1 ? rc(1) : 0.0;

    return {r, LSF, rc2};
}

} 