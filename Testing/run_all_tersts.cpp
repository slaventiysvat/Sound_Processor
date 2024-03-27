//#define GTEST_OS_QURT
#include <gtest/gtest.h>
#include <gmock/gmock.h>
//#include "special_math.h"


using ::testing::AllOf;
using ::testing::Gt;
using ::testing::Lt;
using ::testing::Each;
using ::testing::FloatNear;
using ::testing::FloatEq;

TEST(_EngineString, Basic) {

    //matrix_array Arr_1 = matrix_create(4, 6);
    //s_values(&Arr_1, 1.0);
    //print_matrix(&Arr_1);
    //ASSERT_THAT(Arr_1.s_vector, Each(FloatEq(1.0t)));
    EXPECT_EQ(0, 0);
}

