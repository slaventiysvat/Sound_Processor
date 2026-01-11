#include "eigen/EigenMathApi.hpp"
#include <Eigen/Dense>

extern "C" {

EIGEN_API int eig_dot_f32(const float* a, const float* b, std::size_t n, float* out) {
  if (!a || !b || !out) return 0;
  Eigen::Map<const Eigen::ArrayXf> A(a, (Eigen::Index)n);
  Eigen::Map<const Eigen::ArrayXf> B(b, (Eigen::Index)n);
  *out = (A * B).sum();
  return 1;
}

EIGEN_API int eig_acorr_f32(const float* x, std::size_t n, std::size_t order, float* r_out) {
  if (!x || !r_out || n == 0) return 0;
  Eigen::Map<const Eigen::ArrayXf> X(x, (Eigen::Index)n);
  for (std::size_t k = 0; k <= order; ++k) {
    const Eigen::Index len = (Eigen::Index)(n - k);
    if (len <= 0) { r_out[k] = 0.0f; continue; }
    r_out[k] = (X.head(len) * X.tail(len)).sum();
  }
  return 1;
}

EIGEN_API int eig_lpc_levinson_f32(const float* r, std::size_t order, float* a_out, float* err_out) {
  if (!r || !a_out || order == 0) return 0;
  // класичний Levinson-Durbin (float)
  std::vector<float> a(order+1, 0.0f); // a[0]=1, a[1..p]
  a[0] = 1.0f;
  float E = r[0];
  if (E <= 0.0f) { if (err_out) *err_out = E; std::fill(a_out, a_out+order, 0.0f); return 1; }

  for (std::size_t i=1; i<=order; ++i) {
    float acc = 0.0f;
    for (std::size_t j=1; j<i; ++j) acc += a[j] * r[i - j];
    float kappa = -(r[i] + acc) / E;

    // a_new
    for (std::size_t j=1; j<i; ++j) a[j] = a[j] + kappa * a[i - j];
    a[i] = kappa;
    E = E * (1.0f - kappa*kappa);
    if (E <= 1e-12f) break;
  }
  for (std::size_t i=0; i<order; ++i) a_out[i] = a[i+1];
  if (err_out) *err_out = E;
  return 1;
}

} // extern "C"
