#pragma once
#include <array>
#include <string>
#include <ostream>
#include <typeinfo>
#include <utility>

namespace chronos {
// General utilities.

// Sets c to the sum of a and b, returning the carry. The carry is 0 (safe), 1
// (carry/overflow), or -1 (borrow/underflow). It is safe to reuse an input as
// an output.
//
// Note that a carry of 1 means maxSigned, while a carry of -1 means minSigned,
// which is one past -maxSigned.
//
// TODO: Ideally, there would be compiler-specific versions which either take
// advantage of intrinsics or direct support for 128-bit ints. Worst case,
// there's boost/multiprecision/cpp_int.hpp. For now, we're doing things the
// hard way because it's more educational and because it's isolated enough for
// later performance tuning.
//
// BUG: Using this in CanonRep::create apparently complicates the AST to the
// point where, despite being constexpr, constructing a NaN causes runtime
// calls to be generated. This is true even though no actual calls are made to
// addCarry on that code path.
constexpr int64_t addCarry(int64_t a, int64_t b, int64_t& c) {
  constexpr uint64_t maxSigned = std::numeric_limits<int64_t>::max();
  constexpr uint64_t minSigned = std::numeric_limits<int64_t>::min();
  bool aNeg(a < 0), bNeg(b < 0);
  // Mixed signs can't overflow.
  if (aNeg != bNeg) {
    std::swap(a, b);
    c = a + b;
    return 0;
  }

  // Unsigned overflow is defined, so go unsigned.
  if (aNeg) a = -a, b = -b;
  uint64_t aa = a, bb = b, cc = aa + bb;

  int64_t carry = 0;
  if (aNeg) {
    // If underflow, borrow.
    if (cc > minSigned) {
      cc -= minSigned;
      carry = -1;
    }
    // Adjust sign back to negative.
    c = -int64_t(cc);
    return carry;
  }

  // If overflow, carry.
  if (cc > maxSigned) {
    cc -= maxSigned + 1;
    carry = 1;
  }

  c = int64_t(cc);
  return carry;
}

using namespace std::string_view_literals;

// Adapter to allow any dumpable object to be streamed out.
//
// TODO: Get support for wstream working.
// https://stackoverflow.com/questions/52737760/how-to-define-string-literal-with-character-type-that-depends-on-template-parame
// http://coliru.stacked-crooked.com/a/93b8892ef7e26492
template<typename Dumpable, class CharT, class Traits>
inline auto operator<<(::std::basic_ostream<CharT, Traits>& os,
    const Dumpable& item) -> decltype(item.dump(os), os) {
  return item.dump(os);
}

template<typename Dumpable, class CharT, class Traits>
inline auto operator<<(::std::basic_ostream<CharT, Traits>& os,
    const Dumpable& item) -> decltype(dump(os, item), os) {
  return dump(os, item);
}

// Adapters to allow any stringable object to be streamed out.
//
// Arguably, this is a hack. The right answer would either to look for a
// conversion operator or a to_string member or function. We probably want to
// support string_view, as well as string. But we don't want to cause ambiguity
// with std::to_string/to_wstring when streaming primitive numbers.
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
  // While typeid() works for all compliant compilers, its output is not always
  // human-readable. It happens to be readable in MSVC. Either way it also
  // requires explicit checks to add modifiers.
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
