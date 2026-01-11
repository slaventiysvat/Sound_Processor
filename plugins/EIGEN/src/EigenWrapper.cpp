#include "eigen/EigenWrapper.hpp"
#include <Eigen/Dense>
using namespace eigen;
void EigenWrapper::prepare(double /*fs*/, std::size_t /*block*/){}
void EigenWrapper::process(const float* in, float* out, std::size_t n){
  Eigen::Map<const Eigen::ArrayXf> vin(in,(Eigen::Index)n);
  Eigen::Map<Eigen::ArrayXf> vout(out,(Eigen::Index)n);
  vout = vin;
}
extern "C" {
SP_API sp::ISignalProcessor* sp_create(){ return new EigenWrapper(); }
SP_API void sp_destroy(sp::ISignalProcessor* p){ delete p; }
SP_API const char* sp_name(){ return "EIGEN"; }
SP_API int sp_version_major(){ return 1; }
SP_API int sp_version_minor(){ return 0; }
}
