#pragma once
#include <cstdint>
#include <limits>
#include <utility>
#include <compare>
#include "util.h"

namespace chronos {
// Core types.

// However the scalar time is physically represented, it is exposed canonically
// as a pair of UnitSeconds and UnitPicos.
//
// The UnitSeconds value is 64 bits, but 3 numbers near the extremes are
// reserved to represent special values. See SecondsTraits, below, for details.
//
// The UnitPicos value is nominally 64 bits, but it only takes 40 to express
// 1e12, so any values in excess of this represent at least a full second and
// are therefore normalized back down. (Conveniently, this is well within the
// 53 bits that a double can represent exactly.)
//
// In other words, if a calculation arrives at enough picoseconds to constitute
// one or more seconds, these are carried over to the UnitSeconds, and the
// UnitPicos keeps the remainder.
using UnitSeconds = int64_t;
using UnitPicos = int64_t;
struct UnitValue {
  UnitSeconds s;
  UnitPicos ss;

  // For testing.
  constexpr ::std::strong_ordering operator<=>(const UnitValue& rhs) const
      noexcept {
    if (auto cmp = s <=> rhs.s; cmp != 0) return cmp;
    return ss <=> rhs.ss;
  }
};

// These constants are for an idealized calendar, with no time zones or leap
// days or leap years or anything tricky. They are not a replacement for
// comprehensive civil time support.
constexpr const UnitPicos PicosPerSecond = 1'000'000'000'000;
constexpr const UnitPicos NanosPerSecond = 1'000'000'000;
constexpr const UnitPicos MicrosPerSecond = 1'000'000;
constexpr const UnitPicos MillisPerSecond = 1'000;
constexpr const UnitSeconds SecondsPerMinute = 60;
constexpr const UnitSeconds SecondsPerHour = SecondsPerMinute * 60;
constexpr const UnitSeconds SecondsPerDay = SecondsPerHour * 24;
constexpr const UnitSeconds SecondsPerYear = SecondsPerDay * 365;

// Seconds value categories.
enum class Category { Num, NaN, InfN, InfP };

// TODO: This would be a fine place to try out supporting a conversion more
// canonically.
constexpr const auto& asString(const Category& cat) {
  static constexpr auto CategoryNames =
      make_array("Num"sv, "NaN"sv, "-Inf"sv, "+Inf"sv);
  return CategoryNames[static_cast<int>(cat)];
}

// These traits define how we partition seconds to make room for NaN, negative
// infinity, and positive infinity. These are the values that correspond to the
// categories.
template<typename Units = UnitSeconds>
struct SecondsTraits {
  // Magic values (for 16-bit unit):
  // 7FFF =  32767 posinf
  // 7FFE =  32766 max
  // 0000 =      0
  // FFFF =     -1
  // 8002 = -32766 min
  // 8001 = -32767 neginf
  // 8000 = -32768 nan
  static constexpr const Units InfP = std::numeric_limits<Units>::max();
  static constexpr const Units Max = InfP - 1;
  static constexpr const Units Min = -Max;
  static constexpr const Units InfN = -InfP;
  static constexpr const Units NaN = std::numeric_limits<Units>::min();

  // Resolve special categories under addition.
  static constexpr Category addCategories(
      Category catL, Category catR) noexcept {
    if (catL == Category::Num && catR == Category::Num) return Category::Num;
    if (catL == Category::NaN || catR == Category::NaN) return Category::NaN;
    if (catL != Category::InfP && catL != Category::InfN) return catR;
    if (catR == Category::Num || catR == catL) return catL;
    return Category::NaN;
  }

  // Get category from seconds value.
  static constexpr Category toCategory(UnitSeconds s) noexcept {
    if (s >= InfP) return Category::InfP;
    if (s > InfN) return Category::Num;
    if (s > NaN) return Category::InfN;
    return Category::NaN;
  }
};

// Details of sniffing out seconds()/subseconds() methods.
namespace details {
template<class T, class = void>
struct has_seconds : std::false_type {};

template<class T>
struct has_seconds<T,
    std::void_t<typename decltype(std::declval<T>().seconds())>>
    : std::true_type {};

template<class T, class = void>
struct has_subseconds : std::false_type {};

template<class T>
struct has_subseconds<T,
    std::void_t<typename decltype(std::declval<T>().subseconds())>>
    : std::true_type {};

}; // namespace details

// Sniff out chronological scalars by their unique attributes.
//
// TODO: It would be nice if this detection allowed us to replace the
// boilerplate tuple_element and tuple_size structs with a single set which
// uses SFINAE to constrain itself.
template<typename T>
struct is_scalar_unit : std::bool_constant<details::has_seconds<T>::value &&
                            details::has_subseconds<T>::value &&
                            std::numeric_limits<T>::is_specialized &&
                            std::numeric_limits<T>::has_infinity &&
                            std::numeric_limits<T>::is_exact &&
                            std::numeric_limits<T>::is_bounded &&
                            !std::numeric_limits<T>::is_iec559> {};

template<class T>
constexpr bool is_scalar_unit_v = is_scalar_unit<T>::value;

// TODO: Update the natvis file.

} // namespace chronos
