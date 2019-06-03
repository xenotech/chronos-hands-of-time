#pragma once
#include "ScalarUnitChild.h"

namespace chronos {

// Forward.
template<typename Scalar>
class Moment;

template<typename Scalar>
class Duration;

template<typename T>
struct SpecializationTrait<Duration<T>> {
  template<typename U>
  using Other = Duration<U>;
  using Specialization = T;
};

// Duration between two moments in time.
template<typename Scalar = DefaultScalarUnit>
class Duration : public ScalarUnitChild<Duration<Scalar>> {
public:
  using Parent = ScalarUnitChild<Duration<Scalar>>;

  using Parent::Parent;
  using Parent::operator=;
  using Parent::operator<=>;

  template<typename U,
      typename std::enable_if_t<
          std::is_integral<U>::value && !std::is_class<U>::value, int> = 0>
  constexpr const Duration<>& operator*=(const U& rhs) noexcept {
    // If special, stays same. Else, if multiply by zero, goes zero.
    if (isSpecial()) return *this;
    UnitSeconds m(rhs);
    if (!m) return *this = Duration<>(0);
    UnitValue sss = value();
    bool negS0 = (sss.s < 0), negSS0 = (sss.ss < 0);
    sss.s *= m;
    sss.ss *= m;
    bool negS = (sss.s < 0), negSS = (sss.ss < 0);
    // Check for overflow.
    // TODO: Can we do better when ss overflows?
    if (negS0 != negS) category(negS0 ? Category::InfN : Category::InfP);
    else if (negSS0 != negSS) category(Category::NaN);
    else *this = Duration<>(sss);
    return *this;
  }
};

template<typename ScalarT, typename ScalarU>
constexpr const Duration<> operator+(
    const Duration<ScalarT>& lhs, const Duration<ScalarU>& rhs) noexcept {
  return Duration<>(lhs) += rhs;
}

template<typename ScalarT, typename ScalarU>
constexpr const Duration<> operator-(
    const Duration<ScalarT>& lhs, const Duration<ScalarU>& rhs) noexcept {
  return Duration<>(lhs) -= rhs;
}

template<typename ScalarT, typename U,
    typename std::enable_if_t<
        std::is_integral<U>::value && !std::is_class<U>::value, int> = 0>
constexpr const Duration<> operator*(
    const Duration<ScalarT>& lhs, const U& rhs) noexcept {
  return Duration<>(lhs) *= rhs;
}

template<typename ScalarT, typename U,
    typename std::enable_if_t<
        std::is_integral<U>::value && !std::is_class<U>::value, int> = 0>
constexpr const Duration<> operator*(
    const U& lhs, const Duration<ScalarT>& rhs) noexcept {
  return rhs * lhs;
}

template<typename ScalarT, typename U,
    typename std::enable_if_t<
        std::is_integral<U>::value && !std::is_class<U>::value, int> = 0>
constexpr const Duration<> operator/(
    const Duration<ScalarT>& lhs, const U& rhs) noexcept {
  return Duration<>(lhs) /= rhs;
}

// TODO: Add slow float support.

} // namespace chronos
