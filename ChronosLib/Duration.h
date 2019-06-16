#pragma once
#include "ScalarUnitChild.h"

namespace chronos {
// Forward.
template<typename Scalar>
class Moment;

template<typename Scalar>
class Duration;

namespace details {
template<typename T>
struct ScalarChildTraits<Duration<T>> {
  template<typename U>
  using Other = Duration<U>;
  using Scalar = T;
};

} // namespace details

// Duration between two moments in time.
template<typename Scalar = details::DefaultScalarUnit>
class Duration : public details::ScalarUnitChild<Duration<Scalar>> {
public:
  using Parent = details::ScalarUnitChild<Duration<Scalar>>;

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

template<typename ScalarT, typename U,
    typename std::enable_if_t<std::is_integral_v<U> && !std::is_class_v<U>,
        int> = 0>
constexpr const Duration<> operator*(
    const Duration<ScalarT>& lhs, const U& rhs) noexcept {
  return Duration<>(lhs) *= rhs;
}

template<typename ScalarT, typename U,
    typename std::enable_if_t<std::is_integral_v<U> && !std::is_class_v<U>,
        int> = 0>
constexpr const Duration<> operator*(
    const U& lhs, const Duration<ScalarT>& rhs) noexcept {
  return rhs * lhs;
}

template<typename ScalarT, typename U,
    typename std::enable_if_t<std::is_integral_v<U> && !std::is_class_v<U>,
        int> = 0>
constexpr const Duration<> operator/(
    const Duration<ScalarT>& lhs, const U& rhs) noexcept {
  return Duration<>(lhs) /= rhs;
}

// TODO: Add slow float support.

} // namespace chronos

template<typename Scalar>
class std::numeric_limits<chronos::Duration<Scalar>>
    : public std::numeric_limits<Scalar> {};

// These two enable structured binding.
template<std::size_t N, typename Scalar>
struct std::tuple_element<N, chronos::Duration<Scalar>> {
  using type = decltype(std::get<N>(std::declval<chronos::Duration<Scalar>>()));
};

template<typename Scalar>
struct std::tuple_size<chronos::Duration<Scalar>>
    : public std::integral_constant<std::size_t, 2> {};
