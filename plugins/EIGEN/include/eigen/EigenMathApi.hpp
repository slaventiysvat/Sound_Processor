#pragma once
#include <cstddef>

#if defined(_WIN32)
  #if defined(SP_BUILDING_PLUGIN)
    #define EIGEN_API __declspec(dllexport)
  #else
    #define EIGEN_API __declspec(dllimport)
  #endif
#else
  #define EIGEN_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

// dot: out = sum_i a[i]*b[i]
EIGEN_API int eig_dot_f32(const float* a, const float* b, std::size_t n, float* out);

// autocorr r[k] = sum_{i=0}^{n-1-k} x[i]*x[i+k], k=0..order   (r_out size = order+1)
EIGEN_API int eig_acorr_f32(const float* x, std::size_t n, std::size_t order, float* r_out);

// LPC via Levinson-Durbin (mono):
//  - input: autocorr r[0..order]
//  - output: a[1..order] in 'a_out' (size=order), return 1 on success
EIGEN_API int eig_lpc_levinson_f32(const float* r, std::size_t order, float* a_out, float* err_out);

#ifdef __cplusplus
}
#endif
