#pragma once

#include <Eigen/Dense>
#include <limits>

struct VADParams {
    double Init_Noise = 45.0;   // initial noise estimate
    int N = 0;                  // window size
    int N0 = 0;                 // number of frames for long-term min energy calc
    int Ni = 0;                 // frames used for initialization
    int INIT_COUNT = 0;         // number of frames for parameter update speeds

    struct HPFilter {
        Eigen::VectorXd b;      // numerator coefficients
        Eigen::VectorXd a;      // denominator coefficients (a[0] = 1)
        Eigen::VectorXd mem;    // filter memory
    } HPFilt;

    int LA = 0;                 // look-ahead
    int NF = 0;                 // frame size

    Eigen::VectorXd Wmem;       // window memory (N-NF samples)
    Eigen::VectorXd Window;     // analysis window (length N)
    Eigen::VectorXd LagWindow;  // lag window for autocorrelation
    Eigen::VectorXd LBF_CORR;   // correlation for low band filter

    int FrmCount = 0;           // processed frame counter

    Eigen::VectorXd MeanLSF;    // running mean of LSFs
    double MeanSE = 0.0;        // running mean of full band energy
    double MeanSLE = 0.0;       // running mean of low band energy
    double MeanE = 0.0;         // mean energy during init
    double MeanSZC = 0.0;       // mean zero crossing rate

    int count_sil = 0;          // silence counter
    int count_inert = 0;        // inertial counter for appendix II
    int count_update = 0;       // count of updates to means
    int count_ext = 0;          // extension counter for smoothing step 2
    int less_count = 0;         // number of frames under noise threshold
    int flag = 1;               // state flag used in step 2

    Eigen::Vector2i PrevMarkers = Eigen::Vector2i::Ones(); // previous VAD decisions
    double PrevEnergy = 0.0;    // previous frame energy

    double Prev_MinE = std::numeric_limits<double>::infinity();
    double Next_MinE = std::numeric_limits<double>::infinity();
    Eigen::VectorXd MinE_buffer;  // buffer storing block minimum energies
};

// Main VAD processing function
int vadProcess(const Eigen::VectorXd& frame, VADParams& p);

// Helper functions implemented elsewhere
void vadLpAnalysis(const Eigen::VectorXd& xwin, const VADParams& p,
                   Eigen::VectorXd& r, Eigen::VectorXd& lsf, double& rc2);

int makeDec(double dSLE, double dSE, double SD, double dSZC);

double zcr(const Eigen::VectorXd& x);