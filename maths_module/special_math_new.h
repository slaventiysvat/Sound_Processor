#pragma once

#include <Eigen/Core>

//namespace Eigen {
//	namespace Vector {
//		template <class T>
//		inline void push_back(Eigen::Matrix<T, Eigen::Dynamic, 1>& v, const T d) {
//			Eigen::Matrix<T, Eigen::Dynamic, 1> tmp = v;
//			v.resize(tmp.size() + 1);
//			v.head(tmp.size()) = tmp;
//			v[v.size() - 1] = d;
//		}
//	} // namespace Vector
//} // namespace Eigen


void eigen_matrix_multiply_test();