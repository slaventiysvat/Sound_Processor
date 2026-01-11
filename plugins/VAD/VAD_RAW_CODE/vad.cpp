#include "vad.hpp"
#include <algorithm>
#include <cmath>

using Eigen::VectorXd;

namespace {
constexpr int NOISE = 0;
constexpr int VOICE = 1;
}

int vadProcess(const VectorXd& frame, VADParams& p) {
    bool VAD_APPENDIX_II = true;
    p.FrmCount += 1;
    int frm_count = p.FrmCount;

    // ------------------------------------------------------------
    // High-pass filter of new data using previous state
    // ------------------------------------------------------------
    int nb = p.HPFilt.b.size();
    int na = p.HPFilt.a.size();
    int order = std::max(nb, na) - 1;
    if (p.HPFilt.mem.size() != order) {
        p.HPFilt.mem = VectorXd::Zero(order);
    }
    VectorXd x_new_hp(frame.size());
    for (int n = 0; n < frame.size(); ++n) {
        double x = frame[n] * 32768.0; // scaling as in Matlab reference
        double y = p.HPFilt.b[0] * x + (order > 0 ? p.HPFilt.mem[0] : 0.0);
        for (int i = 1; i < order; ++i) {
            p.HPFilt.mem[i-1] = p.HPFilt.b[i] * x + p.HPFilt.mem[i] - p.HPFilt.a[i] * y;
        }
        if (order > 0) {
            p.HPFilt.mem[order-1] = (nb > order ? p.HPFilt.b[order] * x : 0.0)
                                   - (na > order ? p.HPFilt.a[order] * y : 0.0);
        }
        x_new_hp[n] = y;
    }

    // ------------------------------------------------------------
    // Append new filtered data to filter memory
    // ------------------------------------------------------------
    VectorXd xwin(p.Wmem.size() + x_new_hp.size());
    if (p.Wmem.size() > 0)
        xwin << p.Wmem, x_new_hp;
    else
        xwin = x_new_hp;

    // ------------------------------------------------------------
    // LPC analysis
    // ------------------------------------------------------------
    VectorXd r; VectorXd LSF; double rc2;
    vadLpAnalysis(xwin, p, r, LSF, rc2);

    double Ef = 10.0 * std::log10(r[0] / p.N);
    r = r.head(p.LBF_CORR.size());
    double Elow = r[0] * p.LBF_CORR[0];
    if (r.size() > 1) {
        Elow += 2.0 * (r.tail(r.size() - 1).array() *
                       p.LBF_CORR.tail(p.LBF_CORR.size() - 1).array()).sum();
    }
    double El = 10.0 * std::log10(Elow / p.N);
    double SD = (LSF - p.MeanLSF).squaredNorm();
    int ist = p.N - p.LA - p.NF; // zero-based index of current frame start
    double ZC = zcr(xwin.segment(ist, p.NF + 1));

    // ------------------------------------------------------------
    // Long-term minimum energy
    // ------------------------------------------------------------
    p.Next_MinE = std::min(Ef, p.Next_MinE);
    double MinE = std::min(p.Prev_MinE, p.Next_MinE);
    if (p.MinE_buffer.size() == 0)
        p.MinE_buffer = VectorXd::Constant(p.N0/8, std::numeric_limits<double>::infinity());
    if (frm_count % 8 == 0) {
        // shift buffer and update
        for (int i = 0; i < p.MinE_buffer.size()-1; ++i)
            p.MinE_buffer[i] = p.MinE_buffer[i+1];
        p.MinE_buffer[p.MinE_buffer.size()-1] = p.Next_MinE;
        p.Prev_MinE = p.MinE_buffer.minCoeff();
        p.Next_MinE = std::numeric_limits<double>::infinity();
    }

    int marker = NOISE;

    // ------------------------------------------------------------
    // Initialization of running averages
    // ------------------------------------------------------------
    if (frm_count <= p.Ni) {
        if (Ef <= p.Init_Noise) {
            p.less_count += 1;
            marker = NOISE;
        } else {
            marker = NOISE;
            double NEp = (frm_count - 1) - p.less_count;
            double NE = NEp + 1.0;
            p.MeanE = (p.MeanE * NEp + Ef) / NE;
            p.MeanSZC = (p.MeanSZC * NEp + ZC) / NE;
            p.MeanLSF = (p.MeanLSF * NEp + LSF) / NE;
        }
    }

    if (frm_count >= p.Ni) {
        if (frm_count == p.Ni) {
            if (VAD_APPENDIX_II && p.less_count >= p.Ni) {
                p.FrmCount = 0;
                frm_count = 0;
                p.less_count = 0;
            }
            p.MeanSE = p.MeanE - 10.0;
            p.MeanSLE = p.MeanE - 12.0;
        }

        double dSE = p.MeanSE - Ef;
        double dSLE = p.MeanSLE - El;
        double dSZC = p.MeanSZC - ZC;

        if (Ef < p.Init_Noise) {
            marker = NOISE;
        } else {
            marker = makeDec(dSLE, dSE, SD, dSZC);
        }

        if (VAD_APPENDIX_II) {
            if (marker == VOICE)
                p.count_inert = 0;
            if (marker == NOISE && p.count_inert < 3) {
                p.count_inert += 1;
                marker = VOICE;
            }
        }

        // --------------------------------------------------------
        // Voice activity decision smoothing: Step 1
        // --------------------------------------------------------
        if (p.PrevMarkers[0] == VOICE && marker == NOISE &&
            Ef > p.MeanSE + 2.0 && Ef > p.Init_Noise) {
            marker = VOICE;
        }

        // --------------------------------------------------------
        // Voice activity decision smoothing: Step 2
        // --------------------------------------------------------
        if (p.flag == 1) {
            if (p.PrevMarkers[1] == VOICE && p.PrevMarkers[0] == VOICE &&
                marker == NOISE && std::abs(Ef - p.PrevEnergy) <= 3.0) {
                p.count_ext += 1;
                marker = VOICE;
                if (p.count_ext <= 10)
                    p.flag = 1;
                else {
                    p.flag = 0;
                    p.count_ext = 0;
                }
            }
        } else {
            p.flag = 1;
        }

        if (marker == NOISE)
            p.count_sil += 1;

        // --------------------------------------------------------
        // Voice activity decision smoothing: Step 3
        // --------------------------------------------------------
        if (marker == VOICE && p.count_sil >= 5 &&
            Ef - p.PrevEnergy <= 3.0) {
            marker = NOISE;
            p.count_sil = 0;
            if (VAD_APPENDIX_II) {
                p.count_inert = 6;
            }
        }
        if (marker == VOICE)
            p.count_sil = 0;

        // --------------------------------------------------------
        // Voice activity decision smoothing: Step 4
        // --------------------------------------------------------
        if (Ef < p.MeanSE + 2.0 && p.FrmCount > p.N0 && rc2 < 0.6) {
            marker = NOISE;
        }

        bool TestC;
        if (VAD_APPENDIX_II) {
            TestC = (Ef < p.MeanSE + 3.0 && rc2 < 0.75);
        } else {
            TestC = (Ef < p.MeanSE + 3.0 && rc2 < 0.75 && SD < 0.002532959);
        }
        if (TestC) {
            p.count_update += 1;
            double COEF, COEFZC, COEFSD;
            if (p.count_update < p.INIT_COUNT) {
                COEF = 0.75; COEFZC = 0.8; COEFSD = 0.6;
            } else if (p.count_update < p.INIT_COUNT + 10) {
                COEF = 0.95; COEFZC = 0.92; COEFSD = 0.65;
            } else if (p.count_update < p.INIT_COUNT + 20) {
                COEF = 0.97; COEFZC = 0.94; COEFSD = 0.70;
            } else if (p.count_update < p.INIT_COUNT + 30) {
                COEF = 0.99; COEFZC = 0.96; COEFSD = 0.75;
            } else if (p.count_update < p.INIT_COUNT + 40) {
                COEF = 0.995; COEFZC = 0.99; COEFSD = 0.75;
            } else {
                COEF = 0.995; COEFZC = 0.998; COEFSD = 0.75;
            }
            p.MeanLSF = COEFSD * p.MeanLSF + (1 - COEFSD) * LSF;
            p.MeanSE = COEF * p.MeanSE + (1 - COEF) * Ef;
            p.MeanSLE = COEF * p.MeanSLE + (1 - COEF) * El;
            p.MeanSZC = COEFZC * p.MeanSZC + (1 - COEFZC) * ZC;
        }

        if (frm_count > p.N0 &&
            ((p.MeanSE < MinE && SD < 0.002532959) ||
             p.MeanSE > MinE + 10.0)) {
            p.MeanSE = MinE;
            p.count_update = 0;
        }
    }

    // ------------------------------------------------------------
    // Update state for next frame
    // ------------------------------------------------------------
    p.PrevEnergy = Ef;
    p.PrevMarkers << marker, p.PrevMarkers[0];

    // update window memory
    if (p.NF < xwin.size())
        p.Wmem = xwin.segment(p.NF, xwin.size() - p.NF);
    else
        p.Wmem.resize(0);

    return marker;
}