#pragma once
#include "ScalarUnit.h"

namespace chronos {
namespace details {
// Traits.
template<typename T>
struct ScalarChildTraits {
  template<typename U>
  using Other = U;
  using Scalar = T;
};

// Private child of ScalarUnit, public parent of Child.
//
// The purpose of this class is to factor out the common boilerplate code in
// classes such as Moment and Duration. To do this, it uses a variation of the
// curiously-recurring template pattern with some additional template magic to
// allow sniffing out the specialization of the child.
template<typename Child>
class ScalarUnitChild : private ScalarChildTraits<Child>::Scalar {
public:
  // Types.
  using Scalar = typename ScalarChildTraits<Child>::Scalar;
  using Parent = Scalar;

  template<typename U>
  using Other = typename ScalarChildTraits<Child>::Other<U>;

  // Import the methods whose signature is correct, else override and wrap.
  using Scalar::Scalar;
  using Scalar::category;
  using Scalar::seconds;
  using Scalar::subseconds;
  using Scalar::value;
  using Scalar::InfP;
  using Scalar::InfN;
  using Scalar::NaN;
  using Scalar::Min;
  using Scalar::Max;
  using Scalar::isNumber;
  using Scalar::isSpecial;
  using Scalar::isNaN;
  using Scalar::isInfinite;
  using Scalar::isPositiveInfinity;
  using Scalar::isNegativeInfinity;
  using Scalar::dump;

  template<typename ScalarU>
  constexpr Child& operator=(const Other<ScalarU>& rhs) {
    Parent::operator=(rhs);
    return static_cast<Child&>(*this);
  }

  template<typename ScalarU>
  constexpr ::std::partial_ordering operator<=>(const Other<ScalarU>& rhs) const
      noexcept {
    return Parent::operator<=>(static_cast<const Parent&>(rhs));
  }

  constexpr Child operator-() noexcept {
    return Child(Parent::operator-().value());
  }

  template<typename ScalarU>
  constexpr Child& operator+=(const Other<ScalarU>& rhs) noexcept {
    Parent::operator+=(rhs);
    return static_cast<Child&>(*this);
  }

  template<typename ScalarU>
  constexpr Child& operator-=(const Other<ScalarU>& rhs) noexcept {
    Parent::operator-=(rhs);
    return static_cast<Child&>(*this);
  }

  constexpr Child& operator++() noexcept {
    Parent::operator++();
    return static_cast<Child&>(*this);
  }

  constexpr Child operator++(int) noexcept {
    return Child(Parent::operator++(0).value());
  }

  constexpr Child& operator--() noexcept {
    Parent::operator--();
    return static_cast<Child&>(*this);
  }

  constexpr Child operator--(int) noexcept {
    return Child(Parent::operator--(0).value());
  }

  template<typename U,
      typename std::enable_if_t<std::is_integral_v<U> && !std::is_class_v<U>,
          int> = 0>
  constexpr const Child& operator*=(const U& rhs) noexcept {
    Parent::operator*=(rhs).value();
    return static_cast<Child&>(*this);
  }
};

} // namespace details
} // namespace chronos

template<typename Child>
class std::numeric_limits<chronos::details::ScalarUnitChild<Child>>
    : public std::numeric_limits<
          typename chronos::details::ScalarUnitChild<Child>::Scalar> {};
