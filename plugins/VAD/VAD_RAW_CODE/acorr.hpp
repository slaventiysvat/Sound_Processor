#pragma once
#include <Eigen/Dense>

namespace vad {
	Eigen::VectorXd acorr(const Eigen::VectorXd& x, int Nt);
}