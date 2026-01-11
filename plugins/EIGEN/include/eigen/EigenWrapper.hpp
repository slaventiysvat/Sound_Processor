#pragma once
#include "sp/ISignalProcessor.hpp"
#include "sp/export.hpp"
namespace eigen {
class SP_API EigenWrapper final : public sp::ISignalProcessor {
public:
  void prepare(double fs, std::size_t block) override;
  void process(const float* in, float* out, std::size_t n) override;
};
}
