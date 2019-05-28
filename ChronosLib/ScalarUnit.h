#pragma once
#include <iostream>
#include <compare>
#include "CanonRep.h"
#include "RepAdapter.h"
#include "StreamGuard.h"

namespace chronos {
// Scalar unit to hold absolute and relative scalar chronological values, which
// are then used as base for both Duration and Instant classes.
//
// Templated on a base unit that defines the range, precision, and specific
// representation. See RepAdapter and CanonRep for details.
template<typename Rep = CanonRep<>,
  typename Adapter = RepAdapter<Rep>>
  class ScalarUnit : public SecondsTraits<> {
  public:
    // Types.
    using ScalarUnitT = ScalarUnit<Rep, Adapter>;
    using RepT = Rep;
    using AdapterT = Adapter;

    // Types from adapter. Generally not useful.
    // TODO: See if these can be safely removed.
    using WholesT = typename AdapterT::WholesT;
    using FractionsT = typename AdapterT::FractionsT;
    using WholesTraitsT = SecondsTraits<WholesT>;

    // Public types for ScalarUnit, used for all representations.
    using Seconds = UnitSeconds;
    using Picos = UnitPicos;
    using Both = UnitValue;

  private:
    // Fields.
    Adapter m_adapter;

  public:
    // Ctors.
    constexpr ScalarUnit() noexcept : m_adapter() {}

    template <typename T, typename std::enable_if_t<std::is_integral<T>::value, int> = 0>
    constexpr explicit ScalarUnit(T s, UnitPicos ss = 0) noexcept : m_adapter(UnitSeconds(s), ss) {}

    // Warning: This is not at all performant. It's not even eval'ed as constexpr.
    template <typename T, typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    constexpr explicit ScalarUnit(T sss) noexcept : m_adapter(fromFloat(sss)) {}

    constexpr explicit ScalarUnit(Category cat) noexcept { category(cat); }

    constexpr ScalarUnit(const ScalarUnit&) noexcept = default;

    constexpr ScalarUnit(ScalarUnit&&) noexcept = default;

    template<typename RepU, typename AdapterU>
    constexpr explicit ScalarUnit(const ScalarUnit<RepU, AdapterU>& rhs) : m_adapter(rhs.value()) {}

    constexpr ScalarUnit& operator=(const ScalarUnit& rhs) = default;
    constexpr ScalarUnit& operator=(ScalarUnit&&) noexcept = default;
    constexpr ScalarUnit& operator=(Category cat) { category(cat); return *this; }

    template<typename RepU, typename AdapterU>
    constexpr ScalarUnit& operator=(const ScalarUnit<RepU, AdapterU>& rhs) noexcept {
      m_adapter.value(rhs.value());
      return *this;
    }

    // Categories.
    constexpr Category category() const noexcept { return toCategory(seconds()); }
    constexpr void category(Category cat) { m_adapter.category(cat); }

    // Returns whether it has a numerical value, as opposed to a special one.
    constexpr bool isNumber() const noexcept {
      if (auto s = seconds(); s > InfN && s < InfP) return true;
      return false;
    }

    // Returns whether it has a special, non-numerical value.
    constexpr bool isSpecial() const noexcept { return !isNumber(); }

    // Returns whether it is invalid.
    constexpr bool isNaN() const noexcept { return (seconds() == NaN); }

    // Returns whether it is one of the special infinite values.
    constexpr bool isInfinite() const noexcept {
      if (auto s = seconds(); (s > Max) || (s < Min && s != NaN)) return true;
      return false;
    }

    // Returns whether it is positive infinity.
    constexpr bool isPositiveInfinity() const noexcept { return (seconds() > Max); }

    // Returns whether it is negative infinity.
    constexpr bool isNegativeInfinity() const noexcept {
      if (auto s = seconds(); s < Min && s != NaN) return true;
      return false;
    }

    // Constants.
    static constexpr const ScalarUnit getPositiveInfinity() noexcept { return ScalarUnit(InfP); }
    static constexpr const ScalarUnit getNegativeInfinity() noexcept { return ScalarUnit(InfN); }
    static constexpr const ScalarUnit getNaN() noexcept { return ScalarUnit(+1, -1); }

    // Accessors.
    constexpr Seconds seconds() const noexcept { return m_adapter.seconds(); }
    constexpr Picos subseconds() const noexcept { return m_adapter.subseconds(); }
    constexpr UnitValue value() const noexcept { return m_adapter.value(); }

    // Arithmetic operators.

    // Unary minus negates.
    // Note than NaN remains the same.
    constexpr ScalarUnit operator-() {
      // TODO: Maybe optimize to avoid scaling.
      const auto& sss = value();
      return ScalarUnit(-sss.s, -sss.ss);
    }

    template<typename RepU, typename AdapterU>
    constexpr ::std::partial_ordering operator<=>(const ScalarUnit<RepU, AdapterU>& rhs) const noexcept {
      const auto& sssL = value(), sssR = rhs.value();
      if (sssL.s == NaN || sssR.s == NaN) return 1 <=> 0;
      if (auto cmp = sssL.s <=> sssR.s; cmp != 0) return cmp;
      return sssL.ss <=> sssR.ss;
    }

    template<typename RepU, typename AdapterU>
    constexpr ScalarUnit& operator+=(const ScalarUnit<RepU, AdapterU>& rhs) noexcept {
      auto sssL = value(), sssR = rhs.value();
      // If either is special value, result is special.
      auto catL = toCategory(sssL.s), catR = toCategory(sssR.s);
      if (catL != Category::Num || catR != Category::Num) return *this = addCat(catL, catR);
      bool negL = m_adapter.isNegative(), negR = rhs.m_adapter.isNegative();
      sssL.s += sssR.s; sssL.ss += sssR.ss;
      m_adapter.value(sssL);
      // Same sign means addition, which overflows when sign flips.
      if (negL == negR && m_adapter.isNegative() != negL) {
        *this = negL ? Category::InfN : Category::InfP;
      }
      return *this;
    }

    template<typename RepU, typename AdapterU>
    constexpr ScalarUnit& operator-=(const ScalarUnit<RepU, AdapterU>& rhs) noexcept {
      return (*this) += -ScalarUnit<>(rhs);
    }

#if 0
    // Inc pico or sec?
    ScalarUnit operator++(int);
    ScalarUnit operator--(int);
    ScalarUnit& operator++();
    ScalarUnit& operator--();

    ScalarUnit& operator*=(ScalarUnit other);
    ScalarUnit& operator/=(ScalarUnit other);
    ScalarUnit& operator%=(ScalarUnit other);
#endif

    // I/O.
    std::ostream& dump(std::ostream& os) const noexcept {
      Category cat = category();
      if (cat == Category::Num) {
        auto sss = value();
        if (sss.s < 0 || sss.ss < 0) {
          os << "-"; sss.s = -sss.s; sss.ss = -sss.ss;
        } else if (sss.s > 0) os << "+";
        const auto& guard = makeStreamFillGuard(os, '0');
        os << sss.s << "." << std::setw(12) << sss.ss << "s";
      } else {
        os << cat;
      }

      return os << " [" << m_adapter << "]";
    }

    static constexpr UnitValue fromFloat(double sss) noexcept {
      double s = trunc(sss); double ss = sss - s;
      return UnitValue{ static_cast<UnitSeconds>(s),
        static_cast<UnitSeconds>(ss * PicosPerSecond) };
    }
};

template<typename RepT, typename AdapterT, typename RepU, typename AdapterU>
constexpr const ScalarUnit<> operator+(const ScalarUnit<RepT, AdapterT>& lhs,
  const ScalarUnit<RepU, AdapterU>& rhs) noexcept {
  return ScalarUnit<>(lhs) += rhs;
}

template<typename RepT, typename AdapterT, typename RepU, typename AdapterU>
constexpr const ScalarUnit<> operator-(const ScalarUnit<RepT, AdapterT>& lhs,
  const ScalarUnit<RepU, AdapterU>& rhs) noexcept {
  return ScalarUnit<>(lhs) -= rhs;
}

using DefaultScalarUnit = ScalarUnit<DefaultBaseRep>;
using DefaultAdapter = DefaultScalarUnit::AdapterT;
}

template<typename Rep, typename Adapter>
class std::numeric_limits<chronos::ScalarUnit<Rep, Adapter>> : public std::numeric_limits<Rep> {};
