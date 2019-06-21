#pragma once
#include "Core.h"
#include "StreamGuard.h"

namespace chronos {
// This is the adapter for base represenations used to store absolute and
// relative scalar chronological values.
//
// This representation is conceptually a big integer, but may be implemented by
// anything that allows exact representation within its range. So, for example,
// a fixed-point scheme is fine, but floating point isn't. Consult the
// documentation for std::numeric_limits::is_exact for details.
//
// When relative, it represents a duration, which is a signed offset from an
// unspecified point in time. When absolute, it represents an instant in time,
// which is a signed offset from an implied epoch.
//
// The value is always defined in terms of TAI, so such things as leap seconds
// are the responsibility of time zone conversion, even to UTC. There is no
// such thing as days, or even minutes; those are properties of civil times.
//
// The adapter exposes accessors for seconds() and subseconds(). These are both
// signed 64-bit values. Conceptually, you can view the pair as a single
// 128-bit value with an implied radix point in between the two halves.
//
// Note: It follows from this that the size of the representation cannot exceed
// 128 bits, and may well be less. Given that there's room for enough seconds
// to encode over half a trillion years, this should prove sufficient for the
// foreseeable future.
//
// Note: With two's complement integers, the lack of a negative zero means that
// the minimum has a higher absolute value than the maximum. So, for example, a
// signed byte ranges from -128 to +127. However, negating -128 gives you -128,
// which is an underflow. To avoid this, the lowest value is excluded from the
// range, and is instead used to encode an invalid value. See SecondsTraits and
// below for details.
//
// When the seconds() value is set outside this range, it must be saturated to
// infinity. To implement this, the representation reserves low and high values
// just outside the exposed range and uses these to encode negative and
// positive infinity. However they are encoded, they are always returned as
// InfN and InfP. Positive infinity is defined as
// std::numeric_limits<int64_t>::max(), and negative infinity is its negation,
// which is always std::numeric_limits<int64_t>::min() + 1. Invalid values are
// returned as NaN, which is defined as std::numeric_limits<int64_t>::min().
//
// So, for example, a byte-sized base unit would expose the range [-126, +126]
// reserving -127 and +127 for encoding infinities. If set to 127 or more, it
// would store 127 and return positiveInfinity(). If set to -127 or less, it
// would store -127 and return negativeInfinity(). If made invalid, it would
// store -128 and return NaN(). Note that the invalid state can never be set
// directly through seconds(). It can be set with category(Category::NaN), by
// setting seconds() and subseconds() to conflicting signs, or generally by
// performing any operation that is invalid.
//
// The three special values follow semantics similar to those of the IEEE
// floats. All operations with NaN() yield NaN(). Aside from the NaN()
// propagation, infinities do not change under under addition/subtraction, and
// the result of their multiplication/division is NaN(). Overflow and underflow
// under addition yields infinities. Division by 0 yields positiveInfinity()
// for non-negative values, negativeInfinity() for negative ones. Infinities
// are equal to themselves but always less than or greater than numbers, except
// that all comparisons with NaN() yield false. Negative infinity is less than
// positive infinity. The sum of positiveInfinity() and negativeInfinity() is
// NaN().
//
// Setting seconds() to a special value clears subseconds(). Whenever an
// operation leaves subseconds() with a value whose magnitude exceeds
// PicosPerSecond, the full seconds are carried.
//
// The de facto epoch is 0001-01-01 00:00:00 in the proleptic Gregorian
// calendar.
template<typename Rep>
struct RepAdapter : public SecondsTraits<> {
  using RepT = Rep;
  using RepLimits = std::numeric_limits<Rep>;
  using WholesT = typename Rep::WholesT;
  using FractionsT = typename Rep::FractionsT;

  Rep m_rep;

  constexpr RepAdapter() noexcept : m_rep() {}
  explicit constexpr RepAdapter(const UnitValue& sss) noexcept : m_rep(sss) {}
  explicit constexpr RepAdapter(const Rep& rep) noexcept : m_rep(rep) {}
  constexpr RepAdapter(UnitSeconds s, UnitPicos ss) noexcept : m_rep(s, ss) {}
  constexpr RepAdapter(const RepAdapter&) noexcept = default;

  constexpr RepAdapter& operator=(const RepAdapter&) noexcept = default;

  constexpr UnitSeconds seconds() const noexcept { return m_rep.seconds(); }
  constexpr void seconds(UnitSeconds s) noexcept { m_rep.seconds(s); }

  constexpr UnitPicos subseconds() const noexcept { return m_rep.subseconds(); }
  constexpr void subseconds(UnitPicos p) noexcept { m_rep.subseconds(p); }

  constexpr UnitValue value() const noexcept { return m_rep.value(); }
  constexpr void value(const UnitValue& sss) noexcept { m_rep.value(sss); }
  constexpr void value(UnitSeconds s, UnitPicos ss) noexcept {
    m_rep.value(s, ss);
  }

  constexpr bool isNegative() const noexcept {
    return m_rep.wholes() < 0 || m_rep.fractions() < 0;
  }

  constexpr void category(Category cat) noexcept {
    switch (cat) {
    case Category::Num: value(0, 0); break;
    case Category::NaN: value(+1, -1); break;
    case Category::InfN: seconds(InfN); break;
    case Category::InfP: seconds(InfP); break;
    }
  }

  auto dump(std::ostream& os) const -> decltype(os) {
    // return os << m_rep;
    return Rep(m_rep).dump(os);
  }
};

using DefaultAdapter = RepAdapter<details::DefaultBaseRep>;
} // namespace chronos

template<typename Rep>
class std::numeric_limits<chronos::RepAdapter<Rep>>
    : public std::numeric_limits<Rep> {};

// Support structured binding by providing universal get.
namespace std {
template<std::size_t N, typename Unit>
constexpr auto get(const Unit& su,
    std::enable_if_t<chronos::is_scalar_unit_v<Unit>>* = nullptr) noexcept {
  if constexpr (N == 0)
    return su.seconds();
  else if constexpr (N == 1)
    return su.subseconds();
  else if constexpr (N == -1)
    return UnitValue(su.seconds(), su.subseconds());
};

} // namespace std
