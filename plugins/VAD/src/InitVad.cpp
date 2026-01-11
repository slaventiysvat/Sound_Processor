#include "vad/InitVad.hpp"
#include <cmath>
#include <algorithm>

VADParams::VADParams() = default;

void InitVADPar(double Fs, int Nw, int Nsh, int Ni, VADParams &p) {
    p.Fs = Fs;
    p.Frame_Size = Nw;
    p.Ni = Ni;
    p.INIT_COUNT = 20;
    
    // Initialize noise threshold
    p.In_Noise_threshold.resize(Ni, 0.0);
    
    // High-pass filter coefficients (placeholder - would need proper design)
    p.HPFilt_b.resize(7, 0.0);
    p.HPFilt_a.resize(7, 0.0);
    if (!p.HPFilt_b.empty()) p.HPFilt_b[0] = 1.0;
    if (!p.HPFilt_a.empty()) p.HPFilt_a[0] = 1.0;
    
    // Frame parameters
    p.N = Nw;
    p.NF = Nsh;
    
    // Windowing
    p.Window.resize(Nw, 0.0);
    for (int i = 0; i < Nw; ++i) {
        // Hamming window
        p.Window[i] = 0.54 - 0.46 * std::cos(2.0 * 3.14159265358979323846 * i / (Nw - 1));
    }
    
    // Lag window for autocorrelation
    p.LagWindow.resize(p.M + 1, 0.0);
    for (int i = 0; i <= p.M; ++i) {
        p.LagWindow[i] = 1.0;
    }
    
    // Initialize buffers
    p.Wmem.resize(Nw, 0.0);
    p.LBF_CORR.resize(13, 0.0);
    p.FrmEn.resize(p.INIT_COUNT, 0.0);
    p.MeanLSF.resize(p.M, 0.0);
    p.MinE_buffer.resize(p.INIT_COUNT, 0.0);
    
    // Reset counters
    p.FrmCount = 0;
    p.count_sil = 0;
    p.count_inert = 0;
    p.count_update = 0;
    p.count_ext = 0;
    p.less_count = 0;
    p.flag = 1;
    
    // Reset statistics
    p.MeanSE = 0.0;
    p.MeanSLE = 0.0;
    p.MeanE = 0.0;
    p.MeanSZC = 0.0;
    p.PrevEnergy = 0.0;
    p.Prev_MinE = std::numeric_limits<double>::infinity();
    p.Next_MinE = std::numeric_limits<double>::infinity();
}
