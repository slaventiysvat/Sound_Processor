#pragma once
#include "sp/ISignalProcessor.hpp"
#include "sp/export.hpp"
#include <cstddef>
#include <windows.h>
#include "vad/InitVad.hpp" // <- наш стан/ініт тут, без Eigen у хедері

namespace vad {

	// сигнатури math-API з EIGEN.dll
	using eig_dot_f32_t = int(*)(const float*, const float*, std::size_t, float*);
	using eig_acorr_f32_t = int(*)(const float*, std::size_t, std::size_t, float*);
	using eig_lpc_lev_f32_t = int(*)(const float*, std::size_t, float*, float*);

	class SP_API VADProcessor final : public sp::ISignalProcessor {
	public:
		VADProcessor() = default;
		~VADProcessor() override;

		void prepare(double fs, std::size_t block) override;
		void process(const float* in, float* out, std::size_t n) override;

	private:
		void load_eigen_api();

		// runtime dep
		HMODULE hEigen_ = nullptr;
		eig_dot_f32_t     p_dot_ = nullptr;
		eig_acorr_f32_t   p_acorr_ = nullptr;
		eig_lpc_lev_f32_t p_lpc_ = nullptr;

		// локальний VAD-стан
		VADParams S_;
		double fs_ = 16000.0;
		std::size_t block_ = 0;

		// локальні пороги/параметри
		float energyThresh_ = 1e-4f;
	};

} // namespace vad
