#ifndef STAN_VARIATIONAL_ADVI_PARAMS_FULLRANK__HPP
#define STAN_VARIATIONAL_ADVI_PARAMS_FULLRANK__HPP

#include <stan/math/prim/mat/fun/Eigen.hpp>
#include <stan/math/prim/mat/fun/LDLT_factor.hpp>
#include <stan/math/prim/scal/fun/constants.hpp>

#include <stan/math/prim/mat/meta/get.hpp>
#include <stan/math/prim/arr/meta/get.hpp>
#include <stan/math/prim/mat/meta/length.hpp>
#include <stan/math/prim/mat/meta/is_vector.hpp>
#include <stan/math/prim/mat/meta/is_vector_like.hpp>
#include <stan/math/prim/mat/fun/value_of_rec.hpp>

#include <stan/math/prim/mat/err/check_square.hpp>
#include <stan/math/prim/mat/err/check_lower_triangular.hpp>
#include <stan/math/prim/scal/err/check_size_match.hpp>
#include <stan/math/prim/scal/err/check_not_nan.hpp>

#include <vector>

namespace stan {

  namespace variational {

    class advi_params_fullrank {
    private:
      Eigen::VectorXd mu_;      // Mean of location-scale family
      Eigen::MatrixXd L_chol_;  // Cholesky factor of scale matrix
                                // NOTE: \Sigma = L_chol_ * L_chol_.transpose()
      int dimension_;

    public:
      advi_params_fullrank(const Eigen::VectorXd& mu,
                           const Eigen::MatrixXd& L_chol) :
      mu_(mu), L_chol_(L_chol), dimension_(mu.size()) {
        static const char* function =
          "stan::variational::advi_params_fullrank";

        stan::math::check_size_match(function,
                               "Dimension of mean vector",     dimension_,
                               "Dimension of Cholesky factor", L_chol_.rows() );
        stan::math::check_not_nan(function, "Mean vector", mu_);
        stan::math::check_lower_triangular(function,
                               "Cholesky factor", L_chol_);
      }

      // Accessors
      int dimension() const { return dimension_; }
      const Eigen::VectorXd& mu()     const { return mu_; }
      const Eigen::MatrixXd& L_chol() const { return L_chol_; }

      // Mutators
      void set_mu(const Eigen::VectorXd& mu) {
        static const char* function =
          "stan::variational::advi_params_fullrank::set_mu";

        stan::math::check_size_match(function,
                               "Dimension of input vector", mu.size(),
                               "Dimension of current vector", dimension_);
        stan::math::check_not_nan(function, "Input vector", mu);
        mu_ = mu;
      }

      void set_L_chol(const Eigen::MatrixXd& L_chol) {
        static const char* function =
          "stan::variational::advi_params_fullrank::set_L_chol";

        stan::math::check_size_match(function,
                               "Dimension of mean vector",     dimension_,
                               "Dimension of Cholesky factor", L_chol.rows());
        stan::math::check_lower_triangular(function,
                               "Cholesky factor", L_chol);
        L_chol_ = L_chol;
      }

      // Entropy of normal:
      // 0.5 * dim * (1+log2pi) + 0.5 * log det (L^T L) =
      // 0.5 * dim * (1+log2pi) + sum(log(abs(diag(L))))
      double entropy() const {
        double tmp(0.0);
        double result(
          0.5 * static_cast<double>(dimension_)*(1.0 + stan::math::LOG_TWO_PI));
        for (int d = 0; d < dimension_; ++d) {
          tmp = fabs(L_chol_(d, d));
          if (tmp != 0.0) {
            result += log(tmp);
          }
        }
        return result;
      }

      // Implements S^{-1}(eta) = L*eta + \mu
      Eigen::VectorXd
      loc_scale_transform(const Eigen::VectorXd& eta) const {
        static const char* function = "stan::variational::advi_params_fullrank"
                                      "::loc_scale_transform";

        stan::math::check_size_match(function,
                         "Dimension of input vector", eta.size(),
                         "Dimension of mean vector",  dimension_);
        stan::math::check_not_nan(function, "Input vector", eta);

        return (L_chol_*eta).array() + mu_.array();
      }
    };
  }  // variational
}  // stan

#endif
