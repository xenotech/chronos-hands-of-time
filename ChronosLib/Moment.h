#pragma once
#include "ScalarUnit.h"

namespace chronos {
// Moment in time.
template<typename Scalar = DefaultScalarUnit>
class Moment : public Scalar {
public:
  // Types.
  using MomentT = Interval<Scalar>;
  using ScalarUnitT = Scalar;
  using Parent = Scalar;

  using Seconds = UnitSeconds;
  using Picos = UnitPicos;
  using Value = UnitValue;

  // Import methods whose signature is correct, else override and wrap.
  using Scalar::Scalar;
  using Scalar::category;
  using Scalar::seconds;
  using Scalar::subseconds;
  using Scalar::value;
  using Scalar::Max;
  using Scalar::isNumber;
  using Scalar::isSpecial;
  using Scalar::isNaN;
  using Scalar::isInfinite;
  using Scalar::isPositiveInfinity;
  using Scalar::isNegativeInfinity;
  using Scalar::dump;

  template<typename R>
  constexpr Moment& operator=(const R& rhs) {
    static_cast<Parent&>(*this) = rhs;
    return *this;
  }

  template<typename ScalarU>
  constexpr ::std::partial_ordering operator<=>(
      const Moment<ScalarU>& rhs) const noexcept {
    return Parent::operator<=>(rhs);
  }

  constexpr Moment operator-() noexcept {
    return static_cast<Moment>(-static_cast<Parent&>(*this));
  }

  template<typename ScalarU>
  constexpr Moment& operator+=(const Moment<ScalarU>& rhs) noexcept {
    static_cast<Parent&>(*this) += static_cast<const ScalarU&>(rhs);
    return *this;
  }

  template<typename ScalarU>
  constexpr Moment& operator-=(const Moment<ScalarU>& rhs) noexcept {
    static_cast<Parent&>(*this) -= static_cast<const ScalarU&>(rhs);
    return *this;
  }

  constexpr Moment& operator++() noexcept {
    return static_cast<Moment&>(static_cast<Parent&>(*this).operator++());
  }

  constexpr Moment operator++(int) noexcept {
    return static_cast<Moment>(static_cast<Parent&>(*this).operator++(0));
  }

  constexpr Moment& operator--() noexcept {
    return static_cast<Moment&>(static_cast<Parent&>(*this).operator--());
  }

  constexpr Moment operator--(int) noexcept {
    return static_cast<Moment>(static_cast<Parent&>(*this).operator--(0));
  }
};

template<typename ScalarT, typename ScalarU>
constexpr const Moment<> operator+(
    const Moment<ScalarT>& lhs, const Moment<ScalarU>& rhs) noexcept {
  return Moment<>(lhs) += rhs;
}

template<typename ScalarT, typename ScalarU>
constexpr const Moment<> operator-(
    const Moment<ScalarT>& lhs, const Moment<ScalarU>& rhs) noexcept {
  return Moment<>(lhs) -= rhs;
}

} // namespace chronos
