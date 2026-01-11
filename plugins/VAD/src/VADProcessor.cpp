#include "vad/VADProcessor.hpp"
#include "sp/Logger.hpp"
#include <filesystem>
#include <algorithm>
#include <vector>
#include <cmath>

using namespace vad;

static std::filesystem::path dll_dir_of(HMODULE mod) {
    wchar_t buf[MAX_PATH]; GetModuleFileNameW(mod, buf, MAX_PATH);
    return std::filesystem::path(buf).parent_path();
}

VADProcessor::~VADProcessor() {
    if (hEigen_) { 
        FreeLibrary(hEigen_); 
        hEigen_ = nullptr; 
    }
    PLUGIN_INFO("VAD", "VAD instance destroyed");
}

void VADProcessor::load_eigen_api() {
    if (hEigen_) return;

    HMODULE self = nullptr;
    if (!GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&dll_dir_of), 
        &self)) {
        self = nullptr;
    }
    
    auto dir = dll_dir_of(self);
    auto path = dir / L"EIGEN.dll";

    hEigen_ = LoadLibraryW(path.c_str());
    if (!hEigen_) hEigen_ = LoadLibraryW(L"EIGEN.dll");
    
    if (!hEigen_) {
        PLUGIN_WARN("VAD", "Failed to load EIGEN.dll, using fallback");
        return;
    }

    p_dot_ = reinterpret_cast<eig_dot_f32_t>(GetProcAddress(hEigen_, "eig_dot_f32"));
    p_acorr_ = reinterpret_cast<eig_acorr_f32_t>(GetProcAddress(hEigen_, "eig_acorr_f32"));
    p_lpc_ = reinterpret_cast<eig_lpc_lev_f32_t>(GetProcAddress(hEigen_, "eig_lpc_levinson_f32"));
    
    if (p_dot_ && p_acorr_ && p_lpc_) {
        PLUGIN_INFO("VAD", "EIGEN math API loaded successfully");
    } else {
        PLUGIN_WARN("VAD", "Some EIGEN functions missing, using fallback");
    }
}

void VADProcessor::prepare(double fs, std::size_t block) {
    fs_ = fs; 
    block_ = block;
    
    const int Nw = std::max<int>(int(block_), 320);
    const int Nsh = std::max<int>(int(block_), 320);
    const int Ni = 20;
    InitVADPar(fs_, Nw, Nsh, Ni, S_);

    load_eigen_api();
    
    PLUGIN_INFO("VAD", "Prepared: fs=", fs, " Hz, block=", block, 
                ", frame=", Nw, ", threshold=", energyThresh_);
}

void VADProcessor::process(const float* in, float* out, std::size_t n) {
    if (!in || !out || n == 0) return;

    std::size_t i = 0;
    const std::size_t F = std::max<std::size_t>(S_.NF > 0 ? (std::size_t)S_.NF : 320, 80);
    std::vector<float> r(13, 0.0f);
    std::vector<float> a(12, 0.0f);
    float err = 0.0f;
    
    size_t speech_samples = 0;

    while (i < n) {
        const std::size_t L = std::min<std::size_t>(F, n - i);

        bool isSpeech = false;
        if (p_acorr_ && p_lpc_) {
            if (p_acorr_(in + i, L, 12, r.data())) {
                if (p_lpc_(r.data(), 12, a.data(), &err)) {
                    const float energy = (L > 0) ? (r[0] / float(L)) : 0.0f;
                    isSpeech = (energy >= energyThresh_);
                }
            }
        }
        else {
            double acc = 0.0; 
            for (std::size_t k = 0; k < L; ++k) 
                acc += double(in[i + k]) * double(in[i + k]);
            const float energy = float(acc / double(L));
            isSpeech = (energy >= energyThresh_);
        }

        for (std::size_t k = 0; k < L; ++k) {
            out[i + k] = isSpeech ? 1.0f : 0.0f;
            if (isSpeech) speech_samples++;
        }
        i += L;
    }
    
    if (sp::Logger::instance().get_level() <= sp::LogLevel::DEBUG) {
        float speech_ratio = static_cast<float>(speech_samples) / static_cast<float>(n);
        PLUGIN_DEBUG("VAD", "Processed ", n, " samples, speech: ", 
                     (speech_ratio * 100.0f), "%");
    }
}

// Plugin exports
extern "C" {
    SP_API sp::ISignalProcessor* sp_create() {
        return new VADProcessor();
    }
    
    SP_API void sp_destroy(sp::ISignalProcessor* p) {
        delete static_cast<VADProcessor*>(p);
    }
    
    SP_API const char* sp_name() {
        return "VAD";
    }
    
    SP_API int sp_version_major() {
        return 1;
    }
    
    SP_API int sp_version_minor() {
        return 0;
    }
}
