#pragma once

#include <cstdint>
#include <limits>
#include <utility>
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

struct CategoryAsString {
  static const std::string CategoryNames[];
};

inline const std::string& asString(const Category& cat) {
  return CategoryAsString::CategoryNames[static_cast<int>(cat)];
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
  static constexpr Category addCat(Category catL, Category catR) {
    if (catL == Category::Num && catR == Category::Num) return Category::Num;
    if (catL == Category::NaN || catR == Category::NaN) return Category::NaN;
    if (catL == Category::InfP || catL == Category::InfN) {
      if (catR == Category::Num || catR == catL) return catL;
      return Category::NaN;
    }
    return catR;
  }

  // Get category from seconds value.
  static constexpr Category toCategory(UnitSeconds s) noexcept {
    if (s >= InfP) return Category::InfP;
    if (s > InfN) return Category::Num;
    if (s > NaN) return Category::InfN;
    return Category::NaN;
  }
};

} // namespace chronos