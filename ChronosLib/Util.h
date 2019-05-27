#pragma once
#include <string>
#include <ostream>
#include <typeinfo>

namespace chronos {
// General utilities.

// Adapter to allow any dumpable object to be streamed out.
template<typename Dumpable>
inline auto operator<<(::std::ostream& os, const Dumpable& item) ->
decltype(item.dump(os), os) {
  return item.dump(os);
}

template<typename Dumpable>
inline auto operator<<(::std::ostream& os, const Dumpable& item) ->
decltype(dump(os, item), os) {
  return dump(os, item);
}

// Adapters to allow any stringable object to be streamed out.
template<typename Stringable>
inline auto operator<<(::std::ostream& os, const Stringable& item) -> decltype(item.asString(), os) {
  return os << item.asString();
}

template<typename Stringable>
inline auto operator<<(::std::ostream& os, const Stringable& item) -> decltype(asString(item), os) {
  return os << asString(item);
}

}