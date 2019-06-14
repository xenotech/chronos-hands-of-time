#pragma once
#include <iostream>
#include <type_traits>
#include <iomanip>

namespace chronos {
// Stream state guard utilities.
//
// TODO: Improve wstream support.

// Creates a scope that preserves the stream's flags and restores them on exit.
template<typename Stream>
class StreamFlagsGuard {
public:
  using FlagT = std::ios_base::fmtflags;
  static_assert(
      std::is_base_of_v<std::ios, Stream>, "Must be istream or ostream");

  explicit StreamFlagsGuard(Stream& s) : m_stream(s), m_flagBits(s.flags()) {}
  StreamFlagsGuard(Stream& s, FlagT fb) : StreamFlagsGuard(s) { s.flags(fb); }
  ~StreamFlagsGuard() { m_stream.flags(m_flagBits); };
  Stream& operator*() const { return m_stream; }

private:
  Stream& m_stream;
  FlagT m_flagBits;
};

// Creates a scope that preserves the stream's fill character and restores it
// on exit.
template<class CharT, class Traits>
class StreamFillGuard {
public:
  using OStreamT = std::basic_ostream<CharT, Traits>;

  explicit StreamFillGuard(OStreamT& s) : m_stream(s), m_fillChar(s.fill()) {}
  StreamFillGuard(OStreamT& s, int fc) : m_stream(s), m_fillChar(s.fill(fc)) {}
  ~StreamFillGuard() { m_stream.fill(m_fillChar); };
  OStreamT& operator*() const { return m_stream; }

private:
  OStreamT& m_stream;
  CharT m_fillChar;
};

// Creates a scope that preserves the stream's default width and restores it on
// exit.
template<typename Stream>
class StreamWidthGuard {
public:
  static_assert(
      std::is_base_of_v<std::ios, Stream>, "Must be istream or ostream");

  explicit StreamWidthGuard(Stream& s) : m_stream(s), m_cntWidth(s.width()) {}
  StreamWidthGuard(Stream& s, int w) : m_stream(s), m_cntWidth(s.width(w)) {}
  ~StreamWidthGuard() { m_stream.width(m_cntWidth); };
  Stream& operator*() const { return m_stream; }

private:
  Stream& m_stream;
  int m_cntWidth;
};

} // namespace chronos
