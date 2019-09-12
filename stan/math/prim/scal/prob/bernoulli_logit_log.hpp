#ifndef STAN_MATH_PRIM_SCAL_PROB_BERNOULLI_LOGIT_LOG_HPP
#define STAN_MATH_PRIM_SCAL_PROB_BERNOULLI_LOGIT_LOG_HPP

#include <stan/math/prim/meta.hpp>
#include <stan/math/prim/scal/prob/bernoulli_logit_lpmf.hpp>

namespace stan {
namespace math {

/**
 * @deprecated use <code>bernoulli_logit_lpmf</code>
 */
template <bool propto, typename T_n, typename T_prob>
inline auto bernoulli_logit_log(const T_n& n, const T_prob& theta) {
  return bernoulli_logit_lpmf<propto, T_n, T_prob>(n, theta);
}

/**
 * @deprecated use <code>bernoulli_logit_lpmf</code>
 */
template <typename T_n, typename T_prob>
inline auto bernoulli_logit_log(const T_n& n, const T_prob& theta) {
  return bernoulli_logit_lpmf<T_n, T_prob>(n, theta);
}

}  // namespace math
}  // namespace stan
#endif
