#pragma once
#include <cstddef>
namespace sp {
struct ISignalProcessor {
  virtual ~ISignalProcessor() = default;
  virtual void prepare(double sampleRate, std::size_t blockSize) = 0;
  virtual void process(const float* in, float* out, std::size_t n) = 0;
};
}
