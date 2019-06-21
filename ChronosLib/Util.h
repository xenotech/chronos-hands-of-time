#pragma once
#include <array>
#include <string>
#include <ostream>
#include <typeinfo>
#include <utility>

// TODO: ifdef for MSVC.
#include <intrin.h>

namespace chronos {
// General utilities.

// A note about signed and unsigned wrapping.
//
// Unsigned integers have well-defined wrapping behavior, but wrapping for
// signed integers (which amounts to overflow or underflow) is intentionally
// undefined. As a result, the compiler is allowed to optimize on the basis of
// signed wrapping being "impossible". The fix for this is to do the math
// unsigned, then cast back. This works for all hardware with two's-complement
// integers, which is to say all hardware. If there's any concern about one day
// running on a one's-complement machine or some other weirdness, a static
// assert could be added.

// Do addition with well-defined wrapping.
constexpr int64_t addWrapped(int64_t a, int64_t b) noexcept {
  return static_cast<int64_t>(
      static_cast<uint64_t>(a) + static_cast<uint64_t>(b));
}

// Do subtraction with well-defined wrapping.
constexpr int64_t subWrapped(int64_t a, int64_t b) noexcept {
  return static_cast<int64_t>(
      static_cast<uint64_t>(a) - static_cast<uint64_t>(b));
}

// Sets c to the sum of a and b. If there was underflow or overflow, return
// false. Otherwise, return true.
constexpr bool addSafely(int64_t a, int64_t b, int64_t& c) noexcept {
  // Since signed underflow/overflow is not defined, we use unsigned.
  c = addWrapped(a, b);

  // If the inputs have different signs, safe because overflow/underflow is
  // impossible.
  bool aNeg(a < 0), bNeg(b < 0);
  if (aNeg != bNeg) return true;

  // If the output sign matches the input sign, safe because there was no
  // overflow/underflow.
  bool cNeg(c < 0);
  if (aNeg == cNeg) return true;

  // We overflowed/underflowed, so unsafe.
  return false;
}

// Sets c to the sum of a and b, returning the carry. The carry is 0 (safe), 1
// (carry/overflow), or -1 (borrow/underflow). It is safe to reuse an input as
// an output.
constexpr int64_t addCarry(int64_t a, int64_t b, int64_t& c) noexcept {
  if (addSafely(a, b, c)) return 0;

  if (c < 0) {
    // Overflow, so carry.
    c = subWrapped(c, std::numeric_limits<int64_t>::min());
    return +1;
  } else {
    // Underflow, so borrow.
    c = subWrapped(c, std::numeric_limits<int64_t>::max());
    return -1;
  }
}

// Multiplies a by b, setting cLo to the low half of the answer and returning
// the high half. This makes it easy to check for whether the answer is too
// large to fit entirely in cLo. But note that a negative result has a carry of
// -1, not 0, due to sign extension.
int64_t mul128(int64_t a, int64_t b, int64_t& cLo) {
  // TODO: Implement this conditionally for various compilers. It would be nice
  // if there were a way to make it constexpr without doing piecewise
  // multiplication. Ditto for div128. We could use the code at
  // https://bit.ly/2J1tZEc to do the multiplication in constexpr, but it would
  // be substantially more expensive when it's done at runtime.
  int64_t cHi;
  cLo = _mul128(a, b, &cHi);
  return cHi;
}

// Divides the 128-bit value split between dividendHi and dividendLo by
// divisor, setting quotient and returning the remainder. This makes it easy to
// check for whether the dividend was a multiple of the divisor.
int64_t div128(int64_t dividendHi, int64_t dividendLo, int64_t divisor,
    int64_t& quotient) {
  int64_t remainder;
  quotient = _div128(dividendHi, dividendLo, divisor, &remainder);
  return remainder;
}

using namespace std::string_view_literals;

// Adapter to allow any dumpable object to be streamed out.
template<typename Dumpable>
inline auto operator<<(::std::ostream& os, const Dumpable& item)
    -> decltype(item.dump(os), os) {
  return item.dump(os);
}

template<typename Dumpable>
inline auto operator<<(::std::ostream& os, const Dumpable& item)
    -> decltype(dump(os, item), os) {
  return dump(os, item);
}

// Adapters to allow any stringable object to be streamed out.
//
// Arguably, this is a hack. The right answer would either to look for a
// conversion operator or a to_string member or function. We probably want to
// support string_view, as well as string. But we don't want to cause
// ambiguity with std::to_string/to_wstring when streaming primitive numbers.
//
// TODO: Consider doing the arguably right thing.
template<typename Stringable>
inline auto operator<<(::std::ostream& os, const Stringable& item)
    -> decltype(item.asString(), os) {
  return os << item.asString();
}

template<typename Stringable>
inline auto operator<<(::std::ostream& os, const Stringable& item)
    -> decltype(asString(item), os) {
  return os << asString(item);
}

// MSVC mysteriously fails to support this predefined macro.
#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

// Get qualified type name.
template<class T>
std::string type_name() {
  // While typeid() works for all compliant compilers, its output is not
  // always human-readable. It happens to be readable in MSVC. Either way it
  // also requires explicit checks to add modifiers.
  //
  // For GNU, abi::__cxa_demangle lets you convert the mangled form to
  // human-readable, but it's an ugly, overspecific API.
  //
  // An alternative is to use the __PRETTY_FUNCTION__ macro on a templated
  // function, so that the dependent type is demangled as part of it. The only
  // problem is that MSVC doesn't support that #define by that name, which is
  // why we replace it with __FUNCSIG__ above.
  //
  // The end result is ugly due to duplication, but has enough information to
  // be human-readable. It should only be used for debugging and unit-testing.
  // For the latter, it should only be compared to the return value of another
  // call, never to a hardcoded value.
  std::string r = __PRETTY_FUNCTION__;
  r += ": ";
  using TR = typename std::remove_reference<T>::type;
  r += typeid(TR).name();
  if (std::is_const_v<TR>) r += " const";
  if (std::is_volatile_v<TR>) r += " volatile";
  if (std::is_lvalue_reference_v<T>)
    r += "&";
  else if (std::is_rvalue_reference_v<T>)
    r += "&&";
  return r;
}

// make_array from <experimental/array>
//
// TODO: Remove once it's no longer experimental.
namespace details {
template<class>
struct is_ref_wrapper : std::false_type {};
template<class T>
struct is_ref_wrapper<std::reference_wrapper<T>> : std::true_type {};

template<class T>
using not_ref_wrapper = std::negation<is_ref_wrapper<std::decay_t<T>>>;

template<class D, class...>
struct return_type_helper {
  using type = D;
};
template<class... Types>
struct return_type_helper<void, Types...> : std::common_type<Types...> {
  static_assert(std::conjunction_v<not_ref_wrapper<Types>...>,
      "Types cannot contain reference_wrappers when D is void");
};

template<class D, class... Types>
using return_type = std::array<typename return_type_helper<D, Types...>::type,
    sizeof...(Types)>;
} // namespace details

template<class D = void, class... Types>
constexpr details::return_type<D, Types...> make_array(Types&&... t) {
  return {std::forward<Types>(t)...};
}

} // namespace chronos
