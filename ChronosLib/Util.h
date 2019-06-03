#pragma once
#include <string>
#include <ostream>
#include <typeinfo>

namespace chronos {
// General utilities.

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
  if (std::is_const<TR>::value) r += " const";
  if (std::is_volatile<TR>::value) r += " volatile";
  if (std::is_lvalue_reference<T>::value)
    r += "&";
  else if (std::is_rvalue_reference<T>::value)
    r += "&&";
  return r;
}

} // namespace chronos
