#ifndef STAN_MATH_REV_FUN_LOG1P_EXP_HPP
#define STAN_MATH_REV_FUN_LOG1P_EXP_HPP

#include <stan/math/rev/meta.hpp>
#include <stan/math/rev/core.hpp>
#include <stan/math/prim/fun/inv_logit.hpp>
#include <stan/math/prim/fun/log1p_exp.hpp>

namespace stan {
namespace math {

/**
 * Return the log of 1 plus the exponential of the specified
 * variable.
 */
inline var log1p_exp(const var& a) {
  return make_callback_var(log1p_exp(a.val()), [a](auto& vi) mutable {
    a.adj() += vi.adj() * inv_logit(a.val());
  });
}

/**
 * Return the elementwise log(1 + exp(x))
 *
 * @tparam T type of input
 * @param x input
 * @return Elementwise log(1 + exp(x))
 */
template <typename T, require_var_matrix_t<T>* = nullptr>
inline auto log1p_exp(const T& x) {
  return make_callback_var(log1p_exp(x.val()), [x](const auto& vi) {
    x.adj().array() += vi.adj().array() * inv_logit(x.val().array());
  });
}

}  // namespace math
}  // namespace stan
#endif
