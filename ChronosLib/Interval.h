#pragma once
#include "ScalarUnit.h"

namespace chronos {

// Forward.
template<typename Scalar>
class Moment;

// Interval between two moments in time.
template<typename Scalar = DefaultScalarUnit>
class Interval : private Scalar {
public:
  // Types.
  using IntervalT = Interval<Scalar>;
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
  constexpr Interval& operator=(const R& rhs) {
    static_cast<Parent&>(*this) = rhs;
    return *this;
  }

  template<typename ScalarU>
  constexpr ::std::partial_ordering operator<=>(
      const Interval<ScalarU>& rhs) const noexcept {
    return Parent::operator<=>(rhs);
  }

  constexpr Interval operator-() noexcept {
    return static_cast<Interval>(-static_cast<Parent&>(*this));
  }

  template<typename ScalarU>
  constexpr Interval& operator+=(const Interval<ScalarU>& rhs) noexcept {
    static_cast<Parent&>(*this) += static_cast<const ScalarU&>(rhs);
    return *this;
  }

  template<typename ScalarU>
  constexpr Interval& operator-=(const Interval<ScalarU>& rhs) noexcept {
    static_cast<Parent&>(*this) -= static_cast<const ScalarU&>(rhs);
    return *this;
  }

  constexpr Interval& operator++() noexcept {
    return static_cast<Interval&>(static_cast<Parent&>(*this).operator++());
  }

  constexpr Interval operator++(int) noexcept {
    return static_cast<Interval>(static_cast<Parent&>(*this).operator++(0));
  }

  constexpr Interval& operator--() noexcept {
    return static_cast<Interval&>(static_cast<Parent&>(*this).operator--());
  }

  constexpr Interval operator--(int) noexcept {
    return static_cast<Interval>(static_cast<Parent&>(*this).operator--(0));
  }
};

template<typename ScalarT, typename ScalarU>
constexpr const Interval<> operator+(
    const Interval<ScalarT>& lhs, const Interval<ScalarU>& rhs) noexcept {
  return Interval<>(lhs) += rhs;
}

template<typename ScalarT, typename ScalarU>
constexpr const Interval<> operator-(
    const Interval<ScalarT>& lhs, const Interval<ScalarU>& rhs) noexcept {
  return Interval<>(lhs) -= rhs;
}

} // namespace chronos