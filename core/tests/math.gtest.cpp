/*

File: ./core/tests/math.gtest.cpp
Author: Aldhinn Espinas
Description: This tests the Celerique Engine math functionalities.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/math.h>
#include <gtest/gtest.h>

namespace celerique {
    /// @brief The GTest unit test suite for math.
    class MathUnitTestCpp : public ::testing::Test {};

    TEST_F(MathUnitTestCpp, createVectors) {
        Vec2 someVector1;
        const Vec3 someVector2;
        Vec4 someVector3;

        // Verify initial values.
        for (ArraySize i = 0; i < someVector1.size(); i++) {
            GTEST_ASSERT_EQ(someVector1[i], 0.0F);
        }
        for (ArraySize i = 0; i < someVector2.size(); i++) {
            GTEST_ASSERT_EQ(someVector2[i], 0.0F);
        }
        for (ArraySize i = 0; i < someVector3.size(); i++) {
            GTEST_ASSERT_EQ(someVector3[i], 0.0F);
        }
    }

    TEST_F(MathUnitTestCpp, outOfRangeVectorIndices) {
        // Initializations with more elements that the vector can contain.
        GTEST_TEST_THROW_(
            Vec3 someVec3({1.0f, 0.0f, 3.0f, 0.0f}),
            ::std::out_of_range,
            GTEST_FATAL_FAILURE_
        );

        // Accessing elements beyond capacity.
        Vec4 someVec4;
        GTEST_TEST_THROW_(someVec4[4], ::std::out_of_range, GTEST_FATAL_FAILURE_);

        // Setting elements beyond capacity.
        Vec2 someVec2;
        GTEST_TEST_THROW_(someVec2[2] = 0.3f, ::std::out_of_range, GTEST_FATAL_FAILURE_);
    }

    TEST_F(MathUnitTestCpp, vectorEqualities) {
        Vec3 someVec1 = {1.0f, 2.0f, 1.0f};
        Vec3 someVec2 = {1.0f, 1.0f, 3.0f};

        GTEST_ASSERT_EQ(someVec1, someVec1);
        GTEST_ASSERT_EQ(someVec2, someVec2);
        GTEST_ASSERT_NE(someVec1, someVec2);
        GTEST_ASSERT_NE(someVec2, someVec1);
    }

    TEST_F(MathUnitTestCpp, copyingVectors) {
        Vec3 v1 = {1.0f, 2.0f, 3.0f};
        Vec3 v2 = v1; // Copying.
        GTEST_ASSERT_EQ(v1, v2);
    }

    TEST_F(MathUnitTestCpp, vectorDotProduct) {
        Vec3 someVec1 = {1.0f, 2.0f, 1.0f};
        Vec3 someVec2 = {1.0f, 1.0f, 3.0f};
        GTEST_ASSERT_EQ(someVec1 * someVec2, 6.0f);
        GTEST_ASSERT_EQ(someVec2 * someVec1, 6.0f);

        // Basis vectors should be orthogonal.
        Vec2 e1 = {1.0, 0.0};
        Vec2 e2 = {0.0, 1.0};
        GTEST_ASSERT_EQ(e1 * e2, 0.0f);
        GTEST_ASSERT_EQ(e2 * e1, 0.0f);
    }

    TEST_F(MathUnitTestCpp, matrixEqualities) {
        Mat3x3 some3x3Mat1 = {
            {1.0f, 0.0f, -1.0f},
            {-1.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 2.0f}
        };
        Mat3x3 some3x3Mat2 = {
            {1.0f, 0.0f, -1.0f},
            {-1.0f, 2.0f, 1.0f},
            {0.0f, 1.0f, -2.0f}
        };

        GTEST_ASSERT_EQ(some3x3Mat1, some3x3Mat1);
        GTEST_ASSERT_EQ(some3x3Mat2, some3x3Mat2);
        GTEST_ASSERT_NE(some3x3Mat1, some3x3Mat2);
        GTEST_ASSERT_NE(some3x3Mat2, some3x3Mat1);
    }

    TEST_F(MathUnitTestCpp, createMatrices) {
        Mat2x2 m1;
        const Mat3x3 m2;
        Mat4x4 m3;

        // Verify initial values.
        for (ArraySize i = 0; i < m1.rows(); i++) {
            for (ArraySize j = 0; j < m1.columns(); j++) {
                GTEST_ASSERT_EQ(m1(i,j), 0.0f);
            }
        }
        for (ArraySize i = 0; i < m2.rows(); i++) {
            for (ArraySize j = 0; j < m2.columns(); j++) {
                GTEST_ASSERT_EQ(m2(i,j), 0.0f);
            }
        }
        for (ArraySize i = 0; i < m3.rows(); i++) {
            for (ArraySize j = 0; j < m3.columns(); j++) {
                GTEST_ASSERT_EQ(m3(i,j), 0.0f);
            }
        }
    }

    TEST_F(MathUnitTestCpp, matrixCopying) {
        Mat4x4 mat1 = {
            {1.0f, 2.0f, -1.0f, 0.0f},
            {0.0f, -1.0f, 1.0f, 0.0f},
            {-1.0f, 2.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f, 1.0f}
        };
        Mat4x4 mat2 = mat1;
        GTEST_ASSERT_EQ(mat1, mat2);
    }

    TEST_F(MathUnitTestCpp, outOfRangeMatrixIndices) {
        // Initializations with more elements that the matrix can contain.
        GTEST_TEST_THROW_(
            Mat2x2 some2x2Matrix1({{1.0f, 2.0f}, {0.0f, 3.0f}, {9.1f, 2.0f}}),
            ::std::out_of_range,
            GTEST_FATAL_FAILURE_
        );
        GTEST_TEST_THROW_(
            Mat2x2 some2x2Matrix2({{1.0f, 2.0f}, {0.0f, 3.0f, 1.0f}}),
            ::std::out_of_range,
            GTEST_FATAL_FAILURE_
        );

        // Accessing elements beyond capacity.
        Mat4x4 some4x4Matrix1;
        GTEST_TEST_THROW_(some4x4Matrix1(1, 4), ::std::out_of_range, GTEST_FATAL_FAILURE_);
        GTEST_TEST_THROW_(some4x4Matrix1(5, 0), ::std::out_of_range, GTEST_FATAL_FAILURE_);

        // Setting elements beyond capacity.
        Mat3x3 some3x3Matrix1;
        GTEST_TEST_THROW_(some3x3Matrix1(2, 4) = 0.3f, ::std::out_of_range, GTEST_FATAL_FAILURE_);
        GTEST_TEST_THROW_(some3x3Matrix1(9, 1) = 0.3f, ::std::out_of_range, GTEST_FATAL_FAILURE_);
    }

    TEST_F(MathUnitTestCpp, matrixDotProductVector) {
        Mat2x2 some2x2Mat1 = {
            {1.0f, 1.0f},
            {0.0f, -1.0f}
        };
        Vec2 someVec2Vector1 = {2.0f, 1.0f};
        // To verify:
        //
        // [ 1.0    1.0 ] [ 2.0 ] = [  3.0 ]
        // [ 0.0   -1.0 ] [ 1.0 ]   [ -1.0 ]
        //
        Vec2 expectedVec2Product1 = {3.0f, -1.0f};
        GTEST_ASSERT_EQ(expectedVec2Product1, some2x2Mat1 * someVec2Vector1);

        Mat3x3 some3x3Mat1 = {
            {1.0f, 0.0f, -1.0f},
            {-1.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 2.0f}
        };
        Vec3 someVec3Vector1 = {1.0f, 2.0f, -1.0f};
        // To verify:
        //
        // [  1.0   0.0  -1.0 ] [  1.0 ]   [  2.0 ]
        // [ -1.0   1.0   0.0 ] [  2.0 ] = [  1.0 ]
        // [  0.0   0.0   2.0 ] [ -1.0 ]   [ -2.0 ]
        //
        Vec3 expectedVec3Product1 = {2.0f, 1.0f, -2.0f};
        GTEST_ASSERT_EQ(expectedVec3Product1, some3x3Mat1 * someVec3Vector1);

        // Identity matrices multiplied to any vector will result to the same vector.
        Mat3x3 identity3x3 = {
            {1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 1.0f}
        };
        Vec3 someVec3Vector2 = {2.0f, 1.0f, 1.0f};
        Vec3 someVec4Vector1 = {1.0f, -0.3f, 5.0f};
        GTEST_ASSERT_EQ(identity3x3 * someVec3Vector2, someVec3Vector2);
        GTEST_ASSERT_EQ(identity3x3 * someVec4Vector1, someVec4Vector1);
    }

    TEST_F(MathUnitTestCpp, matrixDotProductMatrix) {
        Mat2x2 some2x2Matrix1 = {
            {0.1f, -1.0f},
            {1.0f, 2.0f}
        };
        Mat2x2 some2x2Matrix2 = {
            {2.0f, 1.0f},
            {0.0f, -2.0f}
        };
        // The expected product when multiplying some2x2Matrix1 * some2x2Matrix2.
        // To verify:
        //
        // [ 0.1  -1.0 ] [ 2.0   1.0 ] = [ 0.2   2.1 ]
        // [ 1.0   2.0 ] [ 0.0  -2.0 ]   [ 2.0  -3.0 ]
        //
        Mat2x2 expected2x2MatProduct1Mat1xMat2 = {
            {0.2f, 2.1f},
            {2.0f, -3.0f}
        };
        // The expected product when multiplying some2x2Matrix2 * some2x2Matrix1.
        // To verify:
        //
        // [ 2.0   1.0 ] [ 0.1  -1.0 ] = [  1.2   0.0 ]
        // [ 0.0  -2.0 ] [ 1.0   2.0 ]   [ -2.0  -4.0 ]
        //
        Mat2x2 expected2x2MatProduct1Mat2xMat1 = {
            {1.2f, 0.0f},
            {-2.0f, -4.0f}
        };
        GTEST_ASSERT_EQ(expected2x2MatProduct1Mat1xMat2, some2x2Matrix1 * some2x2Matrix2);
        GTEST_ASSERT_EQ(expected2x2MatProduct1Mat2xMat1, some2x2Matrix2 * some2x2Matrix1);

        Mat3x3 some3x3Matrix1 = {
            {1.0f, -1.0f, 0.0f},
            {2.0f, -3.0f, 2.0f},
            {-1.0f, 2.0f, -1.0f}
        };
        Mat3x3 some3x3Matrix2 = {
            {1.0f, 0.0f, -1.0f},
            {-1.0f, 2.0f, 1.0f},
            {0.0f, 1.0f, -2.0f}
        };
        // The expected product when multiplying some2x2Matrix1 * some2x2Matrix2.
        // To verify:
        //
        // [  1.0   -1.0    0.0 ] [  1.0   0.0   -1.0 ]   [  2.0  -2.0  -2.0 ]
        // [  2.0   -3.0    2.0 ] [ -1.0   2.0    1.0 ] = [  5.0  -4.0  -9.0 ]
        // [ -1.0    2.0   -1.0 ] [  0.0   1.0   -2.0 ]   [ -3.0   3.0   5.0 ]
        //
        Mat3x3 expected3x3MatProduct1Mat1xMat2 = {
            {2.0f, -2.0f, -2.0f},
            {5.0f, -4.0f, -9.0f},
            {-3.0f, 3.0f, 5.0f}
        };
        // The expected product when multiplying some2x2Matrix2 * some2x2Matrix1.
        // To verify:
        //
        // [  1.0   0.0   -1.0 ] [  1.0   -1.0    0.0 ]   [ 2.0  -3.0  1.0 ]
        // [ -1.0   2.0    1.0 ] [  2.0   -3.0    2.0 ] = [ 2.0  -3.0  3.0 ]
        // [  0.0   1.0   -2.0 ] [ -1.0    2.0   -1.0 ]   [ 4.0  -7.0  4.0 ]
        //
        Mat3x3 expected3x3MatProduct1Mat2xMat1 = {
            {2.0f, -3.0f, 1.0f},
            {2.0f, -3.0f, 3.0f},
            {4.0f, -7.0f, 4.0f}
        };
        GTEST_ASSERT_EQ(expected3x3MatProduct1Mat1xMat2, some3x3Matrix1 * some3x3Matrix2);
        GTEST_ASSERT_EQ(expected3x3MatProduct1Mat2xMat1, some3x3Matrix2 * some3x3Matrix1);

        // Identity matrices multiplied to any vector will result to the same matrix.
        Mat3x3 identity3x3 = {
            {1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 1.0f}
        };
        Mat3x3 some3x3Matrix3 = {
            {2.0f, -9.8f, 3.1f},
            {-0.33f, 0.23f, 1.2f},
            {1.0f, 1.0f, 0.0f}
        };
        Mat3x3 some3x3Matrix4 = {
            {1.0f, -2.0f, -0.1f},
            {0.0f, 3.0f, -1.0f},
            {-1.0f, 0.0f, 2.0f}
        };
        GTEST_ASSERT_EQ(identity3x3 * some3x3Matrix3, some3x3Matrix3);
        GTEST_ASSERT_EQ(some3x3Matrix3 * identity3x3, some3x3Matrix3);
        GTEST_ASSERT_EQ(identity3x3 * some3x3Matrix4, some3x3Matrix4);
        GTEST_ASSERT_EQ(some3x3Matrix4 * identity3x3, some3x3Matrix4);
    }
}