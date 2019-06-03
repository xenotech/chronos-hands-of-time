#pragma once
#include "ScalarUnit.h"

namespace chronos {

// Traits.
template<typename T>
struct SpecializationTrait {
  template<typename U>
  using Other = U;
  using Specialization = T;
};

// Private child of Scalar, public parent of Child.
//
// The purpose of this class is to factor out the common boilerplate code in
// classes such as Moment and Duration. To do this, it uses a variation of the
// curiously-recurring template pattern with some additional template magic to
// allow sniffing out the specialization of the child.
template<typename Child>
class ScalarUnitChild : private SpecializationTrait<Child>::Specialization {
public:
  // Types.
  using Scalar = typename SpecializationTrait<Child>::Specialization;
  using Parent = Scalar;

  template<typename U>
  using Other = typename SpecializationTrait<Child>::Other<U>;

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
  constexpr Child& operator=(const R& rhs) {
    Parent::operator=(rhs);
    return *this;
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
};

} // namespace chronos