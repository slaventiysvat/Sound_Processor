#include "init.hpp"
#include <cmath>

VADParams::VADParams()
    : In_Noise_threshold(), HPFilt_b(7), HPFilt_a(7), HPFilt_mem(), N(0), NF(0),
      Wmem(), Window(), LagWindow(), LBF_CORR(13), FrmEn(), MeanLSF(),
      MinE_buffer() {
  // High-pass filter coefficients
  HPFilt_b << 0.895730417643409, -5.37438250586045, 13.4359562646511,
      -17.9146083528682, 13.4359562646511, -5.37438250586045, 0.895730417643409;
  HPFilt_a << 1.0, -5.77981206364905, 13.9231708465036, -17.8928993556478,
      12.9378695523634, -4.99066192992268, 0.802332981091635;

  // Correlation for a lowpass filter
  LBF_CORR << 0.24017939691329, 0.21398822343783, 0.14767692339633,
      0.07018811903116, 0.00980856433051, -0.02015934721195, -0.02388269958005,
      -0.01480076155002, -0.00503292155509, 0.00012141366508, 0.00119354245231,
      0.00065908718613, 0.00015015782285;
}

void InitVADPar(double Fs, int Nw, int Nsh, int Ni, VADParams &p) {
  const double pi = std::acos(-1.0);

  // initialize constant parameters depending on input
  p.Fs = Fs;
  p.Frame_Size = Nsh;
  p.N = Nw;
  p.NF = Nsh;
  p.Ni = Ni;
  p.In_Noise_threshold = Eigen::ArrayXd::Zero(Ni);

  int LWmem = p.N - p.NF;
  p.Wmem = Eigen::VectorXd::Zero(LWmem);

  // Window calculation
  int LA = p.LA;
  int LB = p.N - LA;
  p.Window.resize(p.N);
  for (int i = 0; i < LB; ++i) {
    p.Window(i) = 0.54 - 0.46 * std::cos(2 * pi * i / (2 * LB - 1));
  }
  for (int i = 0; i < LA; ++i) {
    p.Window(LB + i) = std::cos(2 * pi * i / (4 * LA - 1));
  }

  // Lag window for LP analysis
  double BWExp = 60.0;
  double w0 = 2.0 * pi * BWExp / Fs;
  int NP = p.NP;
  double Wn = 1.0001;
  p.LagWindow.resize(NP + 1);
  p.LagWindow(0) = 1.0;
  for (int i = 1; i <= NP; ++i) {
    p.LagWindow(i) = std::exp(-0.5 * std::pow(w0 * i, 2));
  }
  p.LagWindow /= Wn;

  // initialize variable parameters
  p.FrmCount = 0;
  p.FrmEn =
      Eigen::VectorXd::Constant(p.N0, std::numeric_limits<double>::infinity());
  p.MeanLSF = Eigen::VectorXd::Zero(p.M);
  p.MeanSE = 0.0;
  p.MeanSLE = 0.0;
  p.MeanE = 0.0;
  p.MeanSZC = 0.0;
  p.count_sil = 0;
  p.count_inert = 0;
  p.count_update = 0;
  p.count_ext = 0;
  p.less_count = 0;
  p.flag = 1;
  p.PrevMarkers = Eigen::Array2i(1, 1);
  p.PrevEnergy = 0.0;
  p.Prev_MinE = std::numeric_limits<double>::infinity();
  p.Next_MinE = std::numeric_limits<double>::infinity();
  p.MinE_buffer = Eigen::VectorXd::Constant(
      p.N0 / 8, std::numeric_limits<double>::infinity());
}