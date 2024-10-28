#include "pch.h"

#include <iostream>
#include <Eigen/Dense>

using Eigen::MatrixXd;
using Eigen::VectorXd;

void eigen_matrix_multiply_test() {

	MatrixXd m = MatrixXd::Random(3, 3);
	m = (m + MatrixXd::Constant(3, 3, 1.2)) * 50;
	std::cout << "m =" << std::endl << m << std::endl;
	VectorXd v(3);
	v << 1, 2, 3;
	std::cout << "m * v =" << std::endl << m * v << std::endl;
}
