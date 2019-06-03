#pragma once
#include "ScalarUnitChild.h"

namespace chronos {

// Forward.
template<typename Scalar>
class Moment;

template<typename Scalar>
class Interval;

template<typename T>
struct SpecializationTrait<Interval<T>> {
  template<typename U>
  using Other = Interval<U>;
  using Specialization = T;
};

// Interval between two moments in time.
template<typename Scalar = DefaultScalarUnit>
class Interval : public ScalarUnitChild<Interval<Scalar>> {
public:
  using Parent = ScalarUnitChild<Interval<Scalar>>;

  using Parent::Parent;
  using Parent::operator=;
  using Parent::operator<=>;
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
