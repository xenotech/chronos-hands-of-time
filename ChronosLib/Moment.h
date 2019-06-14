#pragma once
#include "ScalarUnit.h"
#include "Duration.h"

namespace chronos {

namespace details {
template<typename T>
struct ScalarChildTraits<Moment<T>> {
  template<typename U>
  using Other = Moment<U>;
  using Scalar = T;
};

} // namespace details

// Moment in time.
template<typename Scalar = details::DefaultScalarUnit>
class Moment : public details::ScalarUnitChild<Moment<Scalar>> {
public:
  using Parent = details::ScalarUnitChild<Moment<Scalar>>;

  using Parent::Parent;
  using Parent::operator=;
  using Parent::operator<=>;
  Parent operator-() = delete;
  template<typename U>
  Parent operator*=(const U&) = delete;

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

template<typename Scalar>
class std::numeric_limits<chronos::Moment<Scalar>>
    : public std::numeric_limits<Scalar> {};

} // namespace chronos

// These two enable structured binding.
template<std::size_t N, typename Scalar>
struct std::tuple_element<N, chronos::Moment<Scalar>> {
  using type = decltype(std::get<N>(std::declval<chronos::Moment<Scalar>>()));
};

template<typename Scalar>
struct std::tuple_size<chronos::Moment<Scalar>>
    : public std::integral_constant<std::size_t, 2> {};
