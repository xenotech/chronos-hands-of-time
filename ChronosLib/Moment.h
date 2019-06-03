#pragma once
#include "ScalarUnit.h"
#include "Duration.h"

namespace chronos {

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
  Moment operator-() = delete;

  template<typename ScalarU>
  constexpr Moment& operator+=(const Duration<ScalarU>& rhs) noexcept {
    return Parent::operator+=(Moment(rhs.value()));
  }

  template<typename ScalarU>
  constexpr Moment& operator-=(const Duration<ScalarU>& rhs) noexcept {
    return Parent::operator-=(Moment(rhs.value()));
  }
};

template<typename ScalarT, typename ScalarU>
constexpr const Moment<> operator+(
    const Moment<ScalarT>& lhs, const Duration<ScalarU>& rhs) noexcept {
  return Moment<>(lhs) += rhs;
}

template<typename ScalarT, typename ScalarU>
constexpr const Moment<> operator+(
    const Duration<ScalarT>& lhs, const Moment<ScalarU>& rhs) noexcept {
  return rhs + lhs;
}

template<typename ScalarT, typename ScalarU>
constexpr const Moment<> operator-(
    const Moment<ScalarT>& lhs, const Duration<ScalarU>& rhs) noexcept {
  return Moment<>(lhs) -= rhs;
}

template<typename ScalarT, typename ScalarU>
constexpr const Duration<> operator-(
    const Moment<ScalarT>& lhs, const Moment<ScalarU>& rhs) noexcept {
  return Duration<>(lhs.value()) -= Duration<>(rhs.value());
}

//
// template<typename ScalarT, typename ScalarU>
// constexpr const Moment<> operator-(
//    const Moment<ScalarT>& lhs, const Moment<ScalarU>& rhs) noexcept {
//  return Moment<>(lhs) -= rhs;
//};

} // namespace chronos
