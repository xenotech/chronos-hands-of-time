#pragma once
#include <iostream>
#include <compare>
#include <tuple>
#include "CanonRep.h"
#include "RepAdapter.h"
#include "StreamGuard.h"

namespace chronos {
namespace details {
// Scalar unit to hold absolute and relative scalar chronological values, which
// are then used as the base for both the Moment and Duration classes.
//
// Templated on a base unit that defines the range, precision, and specific
// representation. See RepAdapter and CanonRep for details.
template<typename Rep = CanonRep<>,
    template<typename> class Adapter = RepAdapter>
class ScalarUnit : public SecondsTraits<> {
public:
  // Types.
  using ScalarUnitT = ScalarUnit<Rep, Adapter>;
  using RepT = Rep;
  using AdapterT = Adapter<Rep>;

  // Public types for ScalarUnit, used for all representations.
  using Seconds = UnitSeconds;
  using Picos = UnitPicos;
  using Value = UnitValue;

private:
  // Fields.
  AdapterT m_adapter;

public:
  // Ctors.
  constexpr ScalarUnit() noexcept : m_adapter() {}

  // Construct by seconds and optional picoseconds.
  template<typename T,
      typename std::enable_if_t<std::is_integral_v<T> && !std::is_class_v<T>,
          int> = 0>
  constexpr explicit ScalarUnit(T s, UnitPicos ss = 0) noexcept
      : m_adapter(UnitSeconds(s), ss) {}

  // Construct by seconds, then numerator and denominator of picoseconds. Does
  // not detect overflow/underflow.
  template<typename T,
      typename std::enable_if_t<std::is_integral_v<T> && !std::is_class_v<T>,
          int> = 0>
  constexpr explicit ScalarUnit(
      T s, UnitPicos numerator, UnitPicos denominator) noexcept
      : m_adapter(UnitSeconds(s), (numerator * PicosPerSecond) / denominator) {}

  // Warning: This is not at all performant. It's not even eval'ed as
  // constexpr.
  template<typename T,
      typename std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
  constexpr explicit ScalarUnit(T sss) noexcept : m_adapter(fromFloat(sss)) {}

  constexpr explicit ScalarUnit(UnitValue sss) noexcept : m_adapter(sss) {}

  constexpr explicit ScalarUnit(Category cat) noexcept { category(cat); }

  constexpr ScalarUnit(const ScalarUnit&) noexcept = default;

  // TODO: Consider whether it's worth providing a non-templated copy ctor.
  template<typename RepU, template<typename> class AdapterU,
      typename std::enable_if_t<
          !std::is_same_v<ScalarUnitT, ScalarUnit<RepU, AdapterU>>, int> = 0>
  constexpr explicit ScalarUnit(const ScalarUnit<RepU, AdapterU>& rhs) noexcept
      : m_adapter(rhs.value()) {}

  constexpr ScalarUnit& operator=(const ScalarUnit& rhs) noexcept = default;
  constexpr ScalarUnit& operator=(Category cat) noexcept {
    category(cat);
    return *this;
  }

  // TODO: Consider whether it's worth providing a non-templated assign op.
  template<typename RepU, template<typename> class AdapterU,
      typename std::enable_if_t<
          !std::is_same_v<ScalarUnitT, ScalarUnit<RepU, AdapterU>>, int> = 0>
  constexpr ScalarUnit& operator=(
      const ScalarUnit<RepU, AdapterU>& rhs) noexcept {
    m_adapter.value(rhs.value());
    return *this;
  }

  // Categories.
  constexpr Category category() const noexcept { return toCategory(seconds()); }
  constexpr void category(Category cat) noexcept { m_adapter.category(cat); }

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
  constexpr bool isPositiveInfinity() const noexcept {
    return (seconds() > Max);
  }

  // Returns whether it is negative infinity.
  constexpr bool isNegativeInfinity() const noexcept {
    if (auto s = seconds(); s < Min && s != NaN) return true;
    return false;
  }

  // Constants.
  static constexpr const ScalarUnit getPositiveInfinity() noexcept {
    return ScalarUnit(InfP);
  }
  static constexpr const ScalarUnit getNegativeInfinity() noexcept {
    return ScalarUnit(InfN);
  }
  static constexpr const ScalarUnit getNaN() noexcept {
    return ScalarUnit(+1, -1);
  }

  // Accessors.
  constexpr Seconds seconds() const noexcept { return m_adapter.seconds(); }
  constexpr Picos subseconds() const noexcept { return m_adapter.subseconds(); }
  constexpr Value value() const noexcept { return m_adapter.value(); }

  // Arithmetic operators.

  // Unary minus negates.
  // Note than NaN remains the same.
  constexpr ScalarUnit operator-() noexcept {
    // TODO: Maybe optimize to avoid scaling.
    const auto& sss = value();
    return ScalarUnit(-sss.s, -sss.ss);
  }

  template<typename RepU, template<typename> class AdapterU>
  constexpr ::std::partial_ordering operator<=>(
      const ScalarUnit<RepU, AdapterU>& rhs) const noexcept {
    const auto sssL = value(), sssR = rhs.value();
    if (sssL.s == NaN || sssR.s == NaN) return 1 <=> 0;
    if (auto cmp = sssL.s <=> sssR.s; cmp != 0) return cmp;
    return sssL.ss <=> sssR.ss;
  }

  template<typename RepU, template<typename> class AdapterU>
  constexpr ScalarUnit& operator+=(
      const ScalarUnit<RepU, AdapterU>& rhs) noexcept {
    auto sssL = value(), sssR = rhs.value();
    if (auto cat = addCategories(toCategory(sssL.s), toCategory(sssR.s));
        cat != Category::Num)
      return *this = cat;
    sssL.ss += sssR.ss; // TODO: Decide if we need addCarry here.
    // Carry/borrow second on s/ss sign difference.
    if (sssL.ss > 0 && sssL.s < 0)
      sssL.ss -= PicosPerSecond, sssL.s++;
    else if (sssL.ss < 0 && sssL.s > 0)
      sssL.ss += PicosPerSecond, sssL.s--;
    if (auto carry = addCarry(sssL.s, sssR.s, sssL.s); carry)
      *this = (carry < 0) ? Category::InfN : Category::InfP;
    else
      m_adapter.value(sssL);
    return *this;
  }

  // TODO: Factor out dependency on _mul128 and _div128.
  // TODO: Write div and mod. Write divmod first.
  template<typename U,
      typename std::enable_if_t<std::is_integral_v<U> && !std::is_class_v<U>,
          int> = 0>
  constexpr const ScalarUnit<>& operator*=(const U& rhs) noexcept {
    // If special, stays that way.
    if (isSpecial()) return *this;
    // Handle multiplication by zero cheaply, as it is common.
    UnitSeconds m(rhs);
    if (!m) return *this = ScalarUnit<>(0);
    UnitValue sss = value();
    if (!sss.s && !sss.ss) return *this;
    bool negS0 = (sss.s < 0), negSS0 = (sss.ss < 0), negM = (m < 0);
    // Multiply whole seconds.
    if (sss.s) {
      sss.s *= m;
      //  bool negS1 = (sss.s < 0);
      // if (negM && negS0 &&
    }
    // Multiply subseconds, then roll whole seconds over.
    if (sss.ss) {
      int64_t carry, low = _mul128(sss.ss, m, &carry);
      sss.s += low / PicosPerSecond, sss.ss = low % PicosPerSecond;
      // Roll over carry, if any.
      if (carry) {
        UnitSeconds ss1, s1 = _div128(carry, 0, PicosPerSecond, &ss1);
        sss.s += s1, sss.ss += ss1;
      }
    }
    // Check for overflow.
    bool negS = (sss.s < 0), negSS = (sss.ss < 0);
    bool negSExpected = negS0 && !negM;
    if (negS0 != negS)
      category(negS0 ? Category::InfN : Category::InfP);
    else if (negSS0 != negSS)
      category(Category::NaN);
    else
      *this = ScalarUnit<>(sss);
    return *this;
  }


  template<typename RepU, template<typename> class AdapterU>
  constexpr ScalarUnit& operator-=(
      const ScalarUnit<RepU, AdapterU>& rhs) noexcept {
    return (*this) += -ScalarUnit<>(rhs);
  }

  constexpr ScalarUnit& operator++() noexcept {
    return (*this) += ScalarUnit(1);
  }

  constexpr ScalarUnit operator++(int) noexcept {
    ScalarUnit s(*this);
    operator++();
    return s;
  }

  constexpr ScalarUnit& operator--() noexcept {
    return (*this) += ScalarUnit(-1);
  }

  constexpr ScalarUnit operator--(int) noexcept {
    ScalarUnit s(*this);
    operator--();
    return s;
  }

  // I/O.
  template<class CharT, class Traits>
  auto dump(std::basic_ostream<CharT, Traits>& os) -> decltype(os) const {
    Category cat = category();
    if (cat == Category::Num) {
      auto sss = value();
      if (sss.s < 0 || sss.ss < 0) {
        os << "-";
        sss.s = -sss.s;
        sss.ss = -sss.ss;
      } else if (sss.s > 0)
        os << "+";
      const auto& guard = StreamFillGuard(os, '0');
      os << sss.s << "." << std::setw(12) << sss.ss << "s";
    } else {
      os << cat;
    }

    return os << " [" << m_adapter << "]";
  }

  static constexpr UnitValue fromFloat(double sss) noexcept {
    double s = trunc(sss);
    double ss = sss - s;
    return UnitValue{static_cast<UnitSeconds>(s),
        static_cast<UnitSeconds>(ss * PicosPerSecond)};
  }
};

template<typename RepT, template<typename> class AdapterT, typename RepU,
    template<typename> class AdapterU>
constexpr const ScalarUnit<> operator+(const ScalarUnit<RepT, AdapterT>& lhs,
    const ScalarUnit<RepU, AdapterU>& rhs) noexcept {
  return ScalarUnit<>(lhs) += rhs;
}

template<typename RepT, template<typename> class AdapterT, typename RepU,
    template<typename> class AdapterU>
constexpr const ScalarUnit<> operator-(const ScalarUnit<RepT, AdapterT>& lhs,
    const ScalarUnit<RepU, AdapterU>& rhs) noexcept {
  return ScalarUnit<>(lhs) -= rhs;
}

using DefaultScalarUnit = ScalarUnit<DefaultBaseRep>;
using DefaultAdapter = DefaultScalarUnit::AdapterT;

} // namespace details
} // namespace chronos

template<typename RepT, template<typename> class AdapterT>
class std::numeric_limits<chronos::details::ScalarUnit<RepT, AdapterT>>
    : public std::numeric_limits<RepT> {};

// These two enable structured binding.
template<std::size_t N, typename Rep, template<typename> class Adapter>
struct std::tuple_element<N, chronos::details::ScalarUnit<Rep, Adapter>> {
  using type = decltype(
      std::get<N>(std::declval<chronos::details::ScalarUnit<Rep, Adapter>>()));
};

template<typename Rep, template<typename> class Adapter>
struct std::tuple_size<chronos::details::ScalarUnit<Rep, Adapter>>
    : std::integral_constant<std::size_t, 2> {};
