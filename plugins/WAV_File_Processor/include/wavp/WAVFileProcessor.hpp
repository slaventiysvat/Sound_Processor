#pragma once
#include "sp/ISignalProcessor.hpp"
#include "sp/export.hpp"
namespace wavp {
class SP_API WAVFileProcessor final : public sp::ISignalProcessor {
public:
  void prepare(double fs, std::size_t block) override { (void)fs; (void)block; }
  void process(const float* in, float* out, std::size_t n) override { for (size_t i=0;i<n;++i) out[i]=in[i]; }
};
}
extern "C" {
  SP_API void wavfp_print_summary(const char* path);
  SP_API int  wavfp_copy_to(const char* inPath, const char* outPath);
}
