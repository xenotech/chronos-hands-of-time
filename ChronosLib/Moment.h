#pragma once
#include "ScalarUnit.h"

namespace chronos {

// Forward.
template<typename Scalar>
class Interval;

template<typename T>
struct SpecializationTrait<Moment<T>> {
  template<typename U>
  using Other = Moment<U>;
  using Specialization = T;
};

// Moment in time.
template<typename Scalar = DefaultScalarUnit>
class Moment : public ScalarUnitChild<Moment<Scalar>> {
public:
  using Parent = ScalarUnitChild<Moment<Scalar>>;

  using Parent::Parent;
  using Parent::operator=;
  using Parent::operator<=>;
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
};

} // namespace chronos
