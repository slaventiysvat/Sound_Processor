#pragma once

#include <Eigen/Dense>
#include <limits>

struct VADParams {
  // constant parameters
  int Init_Noise = 45;
  int M = 42;  // LP order
  int NP = 42; // autocorrelation order
  double Fs = 0.0;
  int Frame_Size = 0;
  int N0 = 16; // number of frames for long-term min energy calculation
  int Ni = 0;  // number of frames for initialization of running averages
  int INIT_COUNT = 20;
  Eigen::ArrayXd In_Noise_threshold; // size Ni

  Eigen::ArrayXd HPFilt_b; // high-pass filter numerator
  Eigen::ArrayXd HPFilt_a; // high-pass filter denominator
  Eigen::ArrayXd HPFilt_mem;

  int N = 0;   // window size
  int LA = 15; // Look-ahead
  int NF = 0;  // frame size
  Eigen::VectorXd Wmem;
  Eigen::VectorXd Window;
  Eigen::VectorXd LagWindow;
  Eigen::VectorXd LBF_CORR;

  // variable parameters
  int FrmCount = 0;
  Eigen::VectorXd FrmEn;
  Eigen::VectorXd MeanLSF;
  double MeanSE = 0.0;
  double MeanSLE = 0.0;
  double MeanE = 0.0;
  double MeanSZC = 0.0;
  int count_sil = 0;
  int count_inert = 0;
  int count_update = 0;
  int count_ext = 0;
  int less_count = 0;
  int flag = 1;
  Eigen::Array2i PrevMarkers = Eigen::Array2i(1, 1);
  double PrevEnergy = 0.0;
  double Prev_MinE = std::numeric_limits<double>::infinity();
  double Next_MinE = std::numeric_limits<double>::infinity();
  Eigen::VectorXd MinE_buffer;

  VADParams();
};

void InitVADPar(double Fs, int Nw, int Nsh, int Ni, VADParams &p);