#ifdef STAN_OPENCL
#include <stan/math.hpp>
#include <gtest/gtest.h>
#include <vector>

TEST(MathMatrixCL, cols_prim) {
  using stan::math::cols;
  using stan::math::matrix_cl;
  matrix_cl<double> m0;
  EXPECT_EQ(0, cols(m0));

  matrix_cl<double> m1(0, 5);
  EXPECT_EQ(5, cols(m1));

  matrix_cl<double> m2(5, 0);
  EXPECT_EQ(0, cols(m2));

  matrix_cl<double> m3(5, 4);
  EXPECT_EQ(4, cols(m3));

  matrix_cl<double> m4(3, 2);
  EXPECT_EQ(2, cols(m4));
}

TEST(MathMatrixCL, cols_rev) {
  using stan::math::cols;
  using stan::math::matrix_cl;
  using stan::math::to_matrix_cl;
  using stan::math::var_value;
  Eigen::Matrix<double, -1, -1> m0(0, 0);
  var_value<matrix_cl<double>> m0_cl = to_matrix_cl(m0);
  EXPECT_EQ(0, cols(m0_cl));

  Eigen::Matrix<double, -1, -1> m1(0, 5);
  var_value<matrix_cl<double>> m1_cl = to_matrix_cl(m1);
  EXPECT_EQ(5, cols(m1_cl));

  Eigen::Matrix<double, -1, -1> m2(5, 0);
  var_value<matrix_cl<double>> m2_cl = to_matrix_cl(m2);
  EXPECT_EQ(0, cols(m2_cl));

  Eigen::Matrix<double, -1, -1> m3(5, 4);
  var_value<matrix_cl<double>> m3_cl = to_matrix_cl(m3);
  EXPECT_EQ(4, cols(m3_cl));

  Eigen::Matrix<double, -1, -1> m4(3, 2);
  var_value<matrix_cl<double>> m4_cl = to_matrix_cl(m4);
  EXPECT_EQ(2, cols(m4_cl));
}

#endif
