#pragma once
#include <vector>
#include <cstddef>
#include <limits>

struct VADParams {
  int Init_Noise = 45;
  int M = 42;
  int NP = 42;
  double Fs = 0.0;
  int Frame_Size = 0;
  int N0 = 16;
  int Ni = 0;
  int INIT_COUNT = 20;

  // заміна Eigen::* на std::vector<double>
  std::vector<double> In_Noise_threshold; // size Ni
  std::vector<double> HPFilt_b; // size 7
  std::vector<double> HPFilt_a; // size 7
  std::vector<double> HPFilt_mem; // runtime

  int N = 0;
  int LA = 15;
  int NF = 0;
  std::vector<double> Wmem;
  std::vector<double> Window;
  std::vector<double> LagWindow;
  std::vector<double> LBF_CORR; // size 13

  // змінні
  int FrmCount = 0;
  std::vector<double> FrmEn;
  std::vector<double> MeanLSF;
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
  int PrevMarkers0 = 1, PrevMarkers1 = 1;  // замість Eigen::Array2i
  double PrevEnergy = 0.0;
  double Prev_MinE = std::numeric_limits<double>::infinity();
  double Next_MinE = std::numeric_limits<double>::infinity();
  std::vector<double> MinE_buffer;

  VADParams();
};

void InitVADPar(double Fs, int Nw, int Nsh, int Ni, VADParams &p);
