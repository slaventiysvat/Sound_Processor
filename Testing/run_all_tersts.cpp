//#define GTEST_OS_QURT
#include <gtest/gtest.h>
#include <gmock/gmock.h>
//#include "special_math.h"
//#include "s_malloc.h"

#define EXPECT_FLOATS_NEARLY_EQ(expected, actual, thresh) \
        EXPECT_EQ(expected.size(), actual.size()) << "Array sizes differ.";\
        EXPECT_EQ(expected[0].size(), actual[0].size()) << "Array sizes differ.";\
        int size = 0; \
        for (int i = 0; i < expected.size(); i++) \
        { \
            size = expected[i].size(); \
            for (int j = 0; j < expected.size();j++) \
            { \
                EXPECT_NEAR(expected[i][j], actual[i][j], thresh) << "at index i: " << i  << "at index j: " << j; \
            } \
        } \
          

using ::testing::AllOf;
using ::testing::Gt;
using ::testing::Lt;
using ::testing::Each;
using ::testing::FloatNear;
using ::testing::FloatEq;
using ::testing::Pointwise;
using std::vector;
//
//Because each sub - vector is independent, you cannot safely guarantee that they
//all have the same length.This can cause runtime exceptions(if you use at())
//or undefined behavior(if you use operator []).The actual storage area of 
//each sub - vector is independent, and more than likely scattered around the
//heap because of automatic resizing.This means that caching will be
//negatively affected.
//
//The proper way is to emulate multiple dimensions by doing the offset
//calculation yourself.This can be done through the use of a wrapper class or helper function.

//int rows = …, cols = …;
//
//auto I = [cols](int row, int col) {
//    return row * cols + col;
//};
//
//std::vector<double> v(rows* cols);
//
//v[I(5, 3)] = 27.0;

//
//double arr[5][3];
//std::vector<double> v;
//
//v.reserve(5 * 3);
//std::copy_n(&arr[0][0], 5 * 3, std::back_inserter(v));

//int main() {
//
//
//    matrix_array Arr_1 = matrix_create(4, 6);
//    s_values(&Arr_1, 1.0);
//    print_matrix(&Arr_1);
//    return 0;
//}



//void fill2DvectorRawVector(vector<vector<float>> &input, matrix_array resource) {
//
//    float* ArrIn = nullptr;
//    ArrIn = resource.s_vector;
//    int m = resource.columns;
//    int n = resource.rows;
//
//        for (int i = 0; i < n; i++) 
//        {
//            float* pvector_tmp = ArrIn + (i * m);
//            for (int j = 0; j < m; j++) 
//            { 
//                input[i][j] = *(pvector_tmp + j);
//            } 
//        } 
//}
//
//
//TEST(_EngineString, Basic) {
//
//    matrix_array Arr_1 = matrix_create(4, 6);
//    s_values(&Arr_1, 1.0);
//    print_matrix(&Arr_1);
//
//    int rows = 4;
//    int columns = 6;
//    float initialValue = 1.0f;
//
//    std::vector<std::vector<float>> twoDVector(rows, std::vector<float>(columns, initialValue));
//    std::vector<std::vector<float>> twoDVectorIn(rows, std::vector<float>(columns, initialValue));
//    fill2DvectorRawVector(twoDVectorIn, Arr_1);
//
//    auto const max_abs_error = 1 / 1024.f;
//    EXPECT_FLOATS_NEARLY_EQ(twoDVectorIn, twoDVector, max_abs_error);
//    EXPECT_EQ(0, 0);
//}

TEST(_EngineString, Basic) {

    EXPECT_EQ(0, 0);
}