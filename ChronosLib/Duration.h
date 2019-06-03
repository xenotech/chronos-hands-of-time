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

} // namespace chronos
