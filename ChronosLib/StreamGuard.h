#pragma once
#include <iostream>
#include <type_traits>
#include <iomanip>

namespace chronos {
// Creates a scope that preserves the stream's flags and restores them on exit.
template<typename Stream>
class StreamFlagsGuard {
public:
  using FlagT = std::ios_base::fmtflags;
  static_assert(std::is_base_of<std::ios, Stream>::value);

  explicit StreamFlagsGuard(Stream& s) : m_stream(s), m_flagBits(s.flags()) {}
  StreamFlagsGuard(Stream& s, FlagT flagBits) : StreamFlagsGuard(s) {
    s.flags(flagBits);
  }
  ~StreamFlagsGuard() { m_stream.flags(m_flagBits); };
  Stream& operator*() const { return m_stream; }

private:
  Stream& m_stream;
  FlagT m_flagBits;
};

template<typename Stream>
auto makeStreamFlagsGuard(Stream& s) {
  return StreamFlagsGuard<Stream>(s);
}

template<typename Stream>
auto makeStreamFlagsGuard(
    Stream& s, typename StreamFlagsGuard<Stream>::FlagT flagBits) {
  return StreamFlagsGuard<Stream>(s, flagBits);
}

// Creates a scope that preserves the stream's fill character and restores it on
// exit.
template<typename OStream>
class StreamFillGuard {
public:
  using CharT = typename OStream::char_type;
  using TraitsT = typename OStream::traits_type;
  static_assert(
      std::is_base_of<std::basic_ostream<CharT, TraitsT>, OStream>::value);

  explicit StreamFillGuard(OStream& s) : m_stream(s), m_fillChar(s.fill()) {}
  StreamFillGuard(OStream& s, CharT fillChar)
      : m_stream(s), m_fillChar(s.fill(fillChar)) {}
  ~StreamFillGuard() { m_stream.fill(m_fillChar); };
  OStream& operator*() const { return m_stream; }

private:
  OStream& m_stream;
  CharT m_fillChar;
};

template<typename OStream>
auto makeStreamFillGuard(OStream& s) {
  return StreamFillGuard<OStream>(s);
}

template<typename OStream>
auto makeStreamFillGuard(
    OStream& s, typename StreamFillGuard<OStream>::CharT fillChar) {
  return StreamFillGuard<OStream>(s, fillChar);
}

// Creates a scope that preserves the stream's default width and restores it on
// exit.
template<typename Stream>
class StreamWidthGuard {
public:
  static_assert(std::is_base_of<std::ios, Stream>::value);

  explicit StreamWidthGuard(Stream& s) : m_stream(s), m_cntWidth(s.width()) {}
  StreamWidthGuard(Stream& s, int cntWidth)
      : m_stream(s), m_cntWidth(s.width(cntWidth)) {}
  ~StreamWidthGuard() { m_stream.width(m_cntWidth); };
  Stream& operator*() const { return m_stream; }

private:
  Stream& m_stream;
  int m_cntWidth;
};

template<typename Stream>
auto makeStreamWidthGuard(Stream& s) {
  return StreamWidthGuard<Stream>(s);
}

template<typename Stream>
auto makeStreamWidthGuard(Stream& s, int cntWidth) {
  return StreamWidthGuard<Stream>(s, cntWidth);
}

} // namespace chronos