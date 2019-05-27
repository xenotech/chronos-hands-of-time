#pragma once
#include <Ratio>
#include <algorithm>
#include "Core.h"

namespace chronos {
// Canonical representation of linear time.
//
// Stores a signed count of seconds and subseconds, both 64 bits by default.
// Provides overflow, scaling, and saturation handling, but no arithmetic
// operations.
//
// The extreme values of the seconds are reserved for NaN and the infinities:
// see SecondsTraits for details. The subseconds are exposed as a count of
// picoseconds. Note that this picosecond count does not represent the entire
// value, just the part that is under a second.
//
// The subseconds have to be signed because the seconds do not support a
// negative zero. However, not all combinations of signs are possible. The
// signs must either match or the other half must be zero. For convenience, if
// no sign is specified for the subseconds, they take on the sign specified for
// the seconds. However, if the subseconds are negative, the seconds must not
// be positive, else the result is NaN.
//
// Internally, the value is contained in a wholes field and a fractions field.
// By default, they correspond exactly to seconds and subseconds (in
// picoseconds). When they differ, this class scales them appropriately.
// However the values are stored, they are always presented externally as
// seconds and picoseconds. Smaller representations increase the likelihood of
// overflowing to infinity, and a biased epoch may be necessary in order to
// represent recent dates.
//
// The wholes and fractions can be specialized to use different sizes (or even
// omitted) and how they are scaled is controlled by the two ratio
// specializations, not just the two types. For example, when Wholes is an
// int8_t, SecondsToWholes remains 1:1, so values exceeding 127 or -127 will
// saturate. Alternately, if SecondsToWholes were set to secondsPerYear:1, then
// a value of 1 would be scaled up to the number of seconds in a year when
// retrieved using seconds() and scaled down (with rounding) when set.
//
// Likewise, when Fractions is an int8_t, it could only hold up to 127
// picoseconds, which is basically useless. A better choice would be to
// simultaneously set FractionsToSeconds to 100, so that it can store
// hundredths of a second. As all math is done in picoseconds and scaled
// appropriately, it would also be possible to set the FractionsToSeconds to
// 127. However, any choice for this value that is not a power of 10 would
// potentially cause rounding errors.
//
// NOTE: The ability to choose smaller (or void) representations is not fully
// implemented or well-tested. It's a tuning feature, not a fundamental one.
//
// Since the wholes and fractions have a hard-coded fixed point between them,
// there's no way to approportion the bits more flexibly. This is unfortunate,
// because the ideal size for storage is probably a 96-bit int which counts
// 1/64th of a ns. To make this possible, CanonRep is used for the canonical
// representation but you can store values in anything you like, so long as
// RepAdapter is specialized to work with it, as well.
//
// TODO: Modify to allow Wholes or Fractions to be void (or equivalent) and
// ensure that it still works correctly. Finish the scaling code.
template<typename Wholes = UnitSeconds,
  typename Fractions = UnitPicos,
  typename SecondsToWholes = std::ratio<1, 1>,
  typename FractionsToSeconds = std::ratio<PicosPerSecond, 1>>
  class CanonRep : public SecondsTraits<Wholes> {
  public:
    using CanonRepT = CanonRep<Wholes, Fractions, SecondsToWholes, FractionsToSeconds>;
    using WholesT = Wholes;
    using FractionsT = Fractions;
    using SecondsToWholesV = SecondsToWholes;
    using FractionsToSecondsV = FractionsToSeconds;

    static constexpr const FractionsT maxFractions =
      std::numeric_limits<Fractions>::max();
    static constexpr const UnitPicos fractionsPerSecond =
      std::min(PicosPerSecond - 1,
        static_cast<UnitPicos>(maxFractions)) + 1;
    static constexpr const bool usesUnitSeconds =
      std::is_same_v<UnitSeconds, Wholes>;
    static constexpr const bool usesUnitPicos =
      std::is_same_v<UnitPicos, Fractions>;

    static_assert(std::numeric_limits<Wholes>::is_signed,
      "Wholes must be signed");
    static_assert(std::numeric_limits<Wholes>::is_integer,
      "Wholes must be integral");
    static_assert(std::numeric_limits<Fractions>::is_signed,
      "Fractions must be signed");
    static_assert(std::numeric_limits<Fractions>::is_integer,
      "Fractions must be integral");

  private:
    // Fields.
    Wholes m_wholes;
    Fractions m_fractions;

  public:
    // Ctors.
    constexpr CanonRep() noexcept : m_wholes(0), m_fractions(0) {}
    constexpr CanonRep(const CanonRep& unit) noexcept = default;
    constexpr CanonRep(CanonRep&& unit) noexcept = default;
    constexpr explicit CanonRep(UnitSeconds s, UnitPicos ss = 0) noexcept : CanonRep(create(s, ss)) {}
    constexpr explicit CanonRep(const UnitValue& sss) noexcept : CanonRep(create(sss)) {}

    constexpr CanonRep& operator=(const CanonRep& unit) noexcept = default;
    constexpr CanonRep& operator=(CanonRep&& unit) noexcept = default;

    enum class Raw { raw };
    constexpr CanonRep(Raw, Wholes w, Fractions f = 0) noexcept : m_wholes(w), m_fractions(f) {}

    // Properties.
    constexpr UnitSeconds seconds() const noexcept { return calcSeconds(); }
    constexpr void seconds(UnitSeconds s) noexcept { *this = create(s, fractions()); }

    constexpr UnitPicos subseconds() const noexcept { return calcPicos(); }
    constexpr void subseconds(UnitPicos ss) noexcept { *this = create(seconds(), ss); }

    constexpr UnitValue value() const noexcept { return { seconds(), subseconds() }; }
    constexpr void value(UnitSeconds s, UnitPicos ss) noexcept { *this = create(s, ss); }
    constexpr void value(const UnitValue& sss) noexcept { *this = create(sss); }

    // Internal properties.
    constexpr Wholes wholes() const noexcept { return m_wholes; }
    constexpr void wholes(Fractions f) noexcept { m_wholes = f; }

    constexpr Fractions fractions() const noexcept { return m_fractions; }
    constexpr void fractions(Fractions f) noexcept { m_fractions = f; }

    std::ostream& dump(std::ostream& os) const noexcept {
      auto flagScope = makeStreamFlagsGuard(os, std::ios::hex);
      return os << static_cast<UnitSeconds>(wholes()) << "."
        << static_cast<UnitPicos>(fractions())
        << " <" << sizeof(WholesT) << ":" << sizeof(FractionsT) << ">";
    }

  private:
    // Create instance from inputs, adjusting signs first. Cleans up the
    // separate inputs to make sure they're compatible.
    constexpr CanonRep create(UnitSeconds s, UnitPicos ss) noexcept {
      if (s < 0 && ss > 0) ss = -ss;
      else if (s > 0 && ss < 0) s = SecondsTraits<>::NaN;
      return create(UnitValue{ s, ss });
    }

    // Creates instance from inputs, with rollover, scaling, and saturation.
    constexpr CanonRep create(UnitValue sss) noexcept {
      // Nobody puts NaN in a corner.
      if (sss.s == SecondsTraits<>::NaN) {
        if constexpr (!usesUnitSeconds) sss.s = NaN;
        sss.ss = 0;
      } else {
        // Roll over excess subseconds.
        if (sss.ss <= -PicosPerSecond || sss.ss >= PicosPerSecond) {
          sss.s += sss.ss / PicosPerSecond;
          sss.ss %= PicosPerSecond;
        }
        // Saturate to infinity, with scaling.
        if (sss.s > Max) {
          if constexpr (!usesUnitSeconds) sss.s = InfP;
          sss.ss = 0;
        } else if (sss.s < Min) {
          if constexpr (!usesUnitSeconds) sss.s = InfN;
          sss.ss = 0;
        }
      }
      Wholes w = calcWholes(sss.s);
      Fractions f = calcFractions(sss.ss);
      return CanonRep(Raw::raw, w, f);
    }

    static constexpr Wholes calcWholes(UnitSeconds s) noexcept {
      // TODO: Scale.
      return static_cast<Wholes>(s);
    }

    constexpr UnitSeconds calcSeconds() const noexcept {
      UnitSeconds s = m_wholes;
      // If necessary, scale infinities up.
      if constexpr (!usesUnitSeconds) {
        if (m_wholes > Max)
          s = SecondsTraits<>::InfP;
        else if (m_wholes < Min) {
          if (m_wholes == NaN) s = SecondsTraits<>::NaN;
          else s = SecondsTraits<>::InfN;
        }
      }
      return s;
    }

    static constexpr Fractions calcFractions(UnitPicos p) {
      Fractions f;
      if constexpr (usesUnitPicos)
        f = p;
      else {
        // TODO: Scale.
        f = static_cast<Fractions>(p);
      }
      return f;
    }

    constexpr UnitPicos calcPicos() const noexcept {
      UnitPicos p = m_fractions;
      if constexpr (!usesUnitPicos) {
        // TODO: Scale.
      }
      return p;
    }
};

using DefaultBaseRep = CanonRep<UnitSeconds, UnitPicos>;
}

// TODO: This needs testing.
template<typename Wholes, typename Fractions, typename SecondsToWholes,
  typename FractionsToSeconds>
  class std::numeric_limits<chronos::CanonRep<Wholes, Fractions, SecondsToWholes,
  FractionsToSeconds>> {
  public:
    using CanonRepT = chronos::CanonRep<Wholes, Fractions, SecondsToWholes,
      FractionsToSeconds>;
    using SecondsT = typename CanonRepT::WholesT;
    using FractionsT = typename CanonRepT::FractionsT;

    [[nodiscard]] static constexpr CanonRepT(min)() noexcept {
      return CanonRepT(CanonRepT::Raw::raw, CanonRepT::Min, FractionsToSeconds::Num - 1);
    }

    [[nodiscard]] static constexpr CanonRepT(max)() noexcept {
      return CanonRepT(CanonRepT::Raw::raw, CanonRepT::Max, FractionsToSeconds::Num - 1);
    }

    [[nodiscard]] static constexpr CanonRepT lowest() noexcept {
      return (min)();
    }

    [[nodiscard]] static constexpr CanonRepT epsilon() noexcept {
      return CanonRepT(CanonRepT::Raw::raw, 0, FractionsToSeconds::Num - 1);
    }

    [[nodiscard]] static constexpr CanonRepT round_error() noexcept {
      return (epsilon)();
    }

    [[nodiscard]] static constexpr CanonRepT denorm_min() noexcept {
      return 0;
    }

    [[nodiscard]] static constexpr CanonRepT infinity() noexcept {
      return CanonRepT(CanonRepT::Raw::raw, CanonRepT::InfP);
    }

    [[nodiscard]] static constexpr CanonRepT quiet_NaN() noexcept {
      return CanonRepT(CanonRepT::Raw::raw, CanonRepT::NaN);
    }

    [[nodiscard]] static constexpr CanonRepT signaling_NaN() noexcept {
      return CanonRepT(CanonRepT::Raw::raw, CanonRepT::NaN, FractionsToSeconds::Num - 1);
    }

    static constexpr float_denorm_style has_denorm = denorm_absent;
    static constexpr bool has_denorm_loss = false;
    static constexpr bool has_infinity = true;
    static constexpr bool has_quiet_NaN = true;
    static constexpr bool has_signaling_NaN = false;
    static constexpr bool is_bounded = true;
    static constexpr bool is_exact = true;
    static constexpr bool is_iec559 = false;
    static constexpr bool is_integer = false;
    static constexpr bool is_modulo = false;
    static constexpr bool is_signed = true;
    static constexpr bool is_specialized = true;
    static constexpr bool tinyness_before = false;
    static constexpr bool traps = false;
    static constexpr float_round_style round_style = round_toward_zero;
    static constexpr int digits = std::numeric_limits<chronos::UnitSeconds>::digits + std::numeric_limits<chronos::UnitPicos>::digits;
    static constexpr int digits10 = std::numeric_limits<SecondsT>::digits10 + std::numeric_limits<chronos::UnitPicos>::digits10;
    static constexpr int max_digits10 = std::numeric_limits<SecondsT>::max_digits10 +
      std::numeric_limits<chronos::UnitPicos>::max_digits10;
    static constexpr int max_exponent = 0;
    static constexpr int max_exponent10 = 0;
    static constexpr int min_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int radix = 2;
};
