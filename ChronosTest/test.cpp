#include "pch.h"
#include <iostream>
#include "../ChronosLib/CanonRep.h"
#include "../ChronosLib/ScalarUnit.h"
#include "../ChronosLib/Moment.h"

using namespace std;
using namespace chronos;

template<typename Thing>
class Dumpster {
  size_t m_line;
  const Thing& m_thing;
  const char* m_label;

public:
  Dumpster(size_t line, const Thing& thing,
    const char* label) : m_line(line), m_thing(thing), m_label(label) {}

  const std::string asString() { return "This has lower priority than dump"; };

  ostream& dump(ostream& os) const {
    return os << m_line << ": " << m_label << "=" << m_thing;
  }
};

#define DUMPSTER(U) Dumpster(__LINE__, U, #U)
#if 0
#define DUMP(U) cout << DUMPSTER(U) << endl;
#else
#define DUMP(U)
#endif

template<class T> struct sink { typedef void type; };
template<class T> using sink_t=typename sink<T>::type;

template<typename T, typename U, typename = void > struct copy_allowed :std::false_type {};
template<typename T, typename U> struct copy_allowed<T, U,
  sink_t<decltype(T(U(0)))>> : std::true_type {};

// TODO: This does not work.
template<typename T, typename U, typename = void > struct assign_allowed :std::false_type {};
template<typename T, typename U> struct assign_allowed<T, U,
  sink_t<decltype(std::declval<T>().operator=(std::declval<U>()))>> : std::true_type {};

TEST(NoCompile, ChronosTest) {
  static_assert(copy_allowed<int, int>::value);
  static_assert(!copy_allowed<int, std::string > ::value);

  static_assert(copy_allowed<ScalarUnit<>, ScalarUnit<>>::value);
  static_assert(copy_allowed<Duration<>, Duration<>>::value);
  static_assert(copy_allowed<Moment<>, Moment<>>::value);

  static_assert(!copy_allowed<Duration<>, ScalarUnit<>>::value);
  static_assert(!copy_allowed<Duration<>, ScalarUnit<>>::value);

  // std::is_constructible yields the wrong answer for these two.
  static_assert(!copy_allowed<ScalarUnit<>, Duration<>>::value);
  static_assert(!copy_allowed<ScalarUnit<>, Moment<>>::value);

  static_assert(!copy_allowed<Moment<>, Duration<>>::value);
  static_assert(!copy_allowed<Duration<>, Moment<>>::value);

  // TODO: Detect that the commented-out ones fail to compile.
  ScalarUnit<> s1(1), s2(2);
  Duration<> d1(3), d2(4);
  Moment<> m1(5), m2(6), m3(7);
  s1 = s1;
  d1 = d1;
  m1 = m1;
  //* s1 = d1;
  //* d1 = s1;
  //* s1 = m1;
  //* m1 = s1;
  //* m1 = d1;
  //* d1 = m1;

  d1 += d2;
  d1 -= d2;
  d1 = d1 + d2;
  d1 = d1 - d2;
  //* d1 += m1;
  //* d1 -= m1;
  //* d1 = d1 - m1;
  //* d1 = d1 + m1;
  m1 = d1 + m1;
  m1 = m1 + d1;
  //* m1 = d1 - m1;
  m1 = m1 - d1;

  //* m1 = -m2;
  //* EXPECT_EQ(m1.seconds(), -m2.seconds());

  //* m3-=m2;
  m3 -= d1;
  m3 += d1;
  //* m1 = m2 + m3;
  m1 = m2 + d1;
  m2 = d1 + m2;
  d1 = m2 - m1;
  d1 = m1 - m2;

  // Test comparisons.
  bool f;
  f = (s1 < s2);
  f = (m1 < m2);
  f = (d1 < d2);
  //* f = (s1 < m1);
  //* f = (s1 < d1);
  //* f = (m1 < d1);
}

template<typename Unit>
void testCtors() {
  Unit nan(Category::NaN);
  DUMP(nan);
  EXPECT_EQ(nan.category(), Category::NaN);
  EXPECT_TRUE(nan.isNaN());
  EXPECT_FALSE(nan.isNumber());
  EXPECT_TRUE(nan.isSpecial());
  EXPECT_FALSE(nan.isNegativeInfinity());
  EXPECT_FALSE(nan.isInfinite());
  EXPECT_FALSE(nan.isPositiveInfinity());
  Unit zero;
  DUMP(zero);
  EXPECT_EQ(zero.category(), Category::Num);
  EXPECT_EQ(zero.seconds(), 0);
  EXPECT_EQ(zero.subseconds(), 0);
  EXPECT_EQ(zero.value(), UnitValue());
  Unit one(1);
  DUMP(one);
  EXPECT_EQ(one.category(), Category::Num);
  EXPECT_EQ(one.seconds(), 1);
  Unit minusone(-1);
  DUMP(minusone);
  EXPECT_EQ(minusone.category(), Category::Num);
  EXPECT_EQ(minusone.seconds(), -1);
  Unit dupe(one);
  DUMP(one);
  EXPECT_EQ(dupe.category(), Category::Num);
  EXPECT_EQ(dupe.seconds(), 1);
  dupe = zero;
  DUMP(dupe);
  EXPECT_EQ(dupe.category(), Category::Num);
  EXPECT_EQ(dupe.seconds(), 0);
  dupe = std::move(one);
  DUMP(dupe);
  EXPECT_EQ(dupe.category(), Category::Num);
  EXPECT_EQ(dupe.seconds(), 1);
  Unit mupe(std::move(dupe));
  DUMP(mupe);
  EXPECT_EQ(dupe.category(), Category::Num);
  EXPECT_EQ(mupe.seconds(), 1);
  Unit neginf(SecondsTraits<>::InfN);
  DUMP(neginf);
  EXPECT_EQ(neginf.category(), Category::InfN);
  Unit posinf(SecondsTraits<>::InfP);
  DUMP(posinf);
  EXPECT_EQ(posinf.category(), Category::InfP);
}

TEST(Optim, ChronosTest) {
  using Base = CanonRep<UnitSeconds, UnitPicos>;
  Base b;
  cout << b.fractions() << endl;
}

TEST(CtorDefault, ChronosTest) {
  using Base = CanonRep<UnitSeconds, UnitPicos>;
  using Unit = ScalarUnit<Base>;
  Unit u(Category::NaN);
  EXPECT_TRUE(u.isNaN());
  EXPECT_FALSE(u.isNumber());
  EXPECT_FALSE(u.isNegativeInfinity());
  EXPECT_FALSE(u.isInfinite());
  EXPECT_FALSE(u.isPositiveInfinity());

  u = Category::InfN;

  testCtors<Unit>();
  testCtors<ScalarUnit<CanonRep<int8_t, int8_t>>>();
  testCtors<ScalarUnit<CanonRep<int16_t, int16_t>>>();
  testCtors<Duration<>>();
  testCtors<Moment<>>();
}

TEST(CtorFloat, ChronosTest) {
  using Unit = ScalarUnit<>;
  Unit a, b;
  a = Unit(1);
  b = Unit(1.0);
  EXPECT_EQ(a, b); DUMP(a); DUMP(b);
  a = Unit(-1);
  b = Unit(-1.0);
  EXPECT_EQ(a, b); DUMP(a); DUMP(b);
  a = Unit(1, 1, 2);
  b = Unit(1.5);
  EXPECT_EQ(a, b); DUMP(a); DUMP(b);
  a = Unit(-1, 1, 2);
  b = Unit(-1.5);
  EXPECT_EQ(a, b); DUMP(a); DUMP(b);
  a = Unit(1, 250000000000);
  b = Unit(1.250000000000);
  EXPECT_EQ(a, b); DUMP(a); DUMP(b);
  a = Unit(1, 1);
  b = Unit(1.000000000001);
  EXPECT_EQ(a, b); DUMP(a); DUMP(b);
  a = Unit(1, 1);
  b = Unit(1.000000000001);
  EXPECT_EQ(a, b); DUMP(a); DUMP(b);
  a = Unit(Unit::Max);
  b = Unit(double(Unit::Max));
  EXPECT_TRUE(b.isNaN());
  EXPECT_NE(a, b); DUMP(a); DUMP(b);
}

TEST(CtorDefaultCopyAB, ChronosTest) {
  using UnitA = ScalarUnit<CanonRep<int8_t, int8_t>>;
  using UnitB = ScalarUnit<CanonRep<int16_t, int16_t>>;
  UnitA a(1);
  UnitB b(2);
  EXPECT_EQ(a.seconds(), 1);
  EXPECT_EQ(b.seconds(), 2);
  DUMP(a);
  DUMP(b);
  a = b;
  EXPECT_EQ(a.seconds(), 2);
  EXPECT_EQ(b.seconds(), 2);
  DUMP(a);
  DUMP(b);
  a = UnitB(3);
  b = UnitA(4);
  DUMP(a);
  DUMP(b);
  EXPECT_EQ(a.seconds(), 3);
  EXPECT_EQ(b.seconds(), 4);
  UnitA c(b);
  DUMP(c);
  a.category(Category::NaN);
  b = a;
  EXPECT_TRUE(b.isNaN());
  UnitA::Seconds sa = SecondsTraits<UnitA::AdapterT::WholesT>::InfP;
  sa = SecondsTraits<UnitA::AdapterT::WholesT>::Max;
  UnitSeconds s = sa;
  a = UnitA(sa);
  DUMP(a);
  b = UnitB(126);
  a = b;
  DUMP(a);
  EXPECT_TRUE(a.isNumber());
  b = UnitB(127);
  a = b;
  DUMP(a);
  EXPECT_TRUE(a.isPositiveInfinity());
  b = UnitB(128);
  a = b;
  DUMP(a);
  EXPECT_TRUE(a.isPositiveInfinity());
}

TEST(SpecialAddition, ChronosTest) {
  using Unit = ScalarUnit<>;
  EXPECT_EQ(Unit::addCategories(Category::Num, Category::Num), Category::Num);
  EXPECT_EQ(Unit::addCategories(Category::Num, Category::NaN), Category::NaN);
  EXPECT_EQ(Unit::addCategories(Category::NaN, Category::Num), Category::NaN);
  EXPECT_EQ(Unit::addCategories(Category::NaN, Category::InfP), Category::NaN);
  EXPECT_EQ(Unit::addCategories(Category::NaN, Category::InfN), Category::NaN);
  EXPECT_EQ(Unit::addCategories(Category::InfP, Category::NaN), Category::NaN);
  EXPECT_EQ(Unit::addCategories(Category::InfN, Category::NaN), Category::NaN);
  EXPECT_EQ(Unit::addCategories(Category::InfP, Category::Num), Category::InfP);
  EXPECT_EQ(Unit::addCategories(Category::InfP, Category::InfP), Category::InfP);
  EXPECT_EQ(Unit::addCategories(Category::InfP, Category::InfN), Category::NaN);
  EXPECT_EQ(Unit::addCategories(Category::InfN, Category::Num), Category::InfN);
  EXPECT_EQ(Unit::addCategories(Category::InfN, Category::InfN), Category::InfN);
  EXPECT_EQ(Unit::addCategories(Category::InfN, Category::InfP), Category::NaN);
  EXPECT_EQ(Unit::addCategories(Category::Num, Category::InfP), Category::InfP);
  EXPECT_EQ(Unit::addCategories(Category::Num, Category::InfN), Category::InfN);
}

template <typename Unit>
void testCompare() {
  EXPECT_NE(Unit(Category::NaN), Unit(Category::NaN));
  EXPECT_EQ(Unit(Category::InfP), Unit(Category::InfP));
  EXPECT_EQ(Unit(Category::InfN), Unit(Category::InfN));
  EXPECT_LT(Unit(Category::InfN), Unit(Category::InfP));
  EXPECT_EQ(Unit(0), Unit(0));
  EXPECT_EQ(Unit(1), Unit(1));
  EXPECT_LT(Unit(0), Unit(1));
  EXPECT_LT(Unit(-1), Unit(0));
  EXPECT_LT(Unit(0), Unit(0, 1));
  EXPECT_LT(Unit(0, -1), Unit(0));
  EXPECT_LT(Unit(-1, 1), Unit(-1));
  EXPECT_EQ(Unit(-1, 1), Unit(-1, -1));
  EXPECT_TRUE(Unit(1, -1).isNaN());
  EXPECT_EQ(Unit(0, PicosPerSecond), Unit(1));
  EXPECT_EQ(Unit(0, PicosPerSecond / 2), Unit(0, 1, 2));
  EXPECT_EQ(Unit(0, -1, 2), Unit(0, 1, -2));
  EXPECT_EQ(Unit(0, 1, 2), Unit(0, -1, -2));
}

TEST(ScalarCompare, ChronosTest) {
  testCompare<ScalarUnit<>>();
  testCompare<Duration<>>();
  testCompare<Moment<>>();
}

template <typename Unit>
void testAdd() {
  Unit a, b, c, d;

  // NaN plus.
  a = Unit(Category::NaN);
  b = Unit(Category::NaN);
  c = a + b;
  // NaN is never equal to anything.
  EXPECT_NE(a, b);
  EXPECT_TRUE(c.isNaN());
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(Category::NaN);
  b = Unit(1);
  c = a + b;
  EXPECT_TRUE(c.isNaN());
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(1);
  b = Unit(Category::NaN);
  c = a + b;
  EXPECT_TRUE(c.isNaN());
  DUMP(a); DUMP(b); DUMP(c);

  // NaN minus.
  a = Unit(Category::NaN);
  b = Unit(Category::NaN);
  c = a - b;
  EXPECT_TRUE(c.isNaN());
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(Category::NaN);
  b = Unit(1);
  c = a - b;
  EXPECT_TRUE(c.isNaN());
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(1);
  b = Unit(Category::NaN);
  c = a - b;
  EXPECT_TRUE(c.isNaN());
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(-Unit::Max + 1);
  b = Unit(-1);
  c = a + b;
  EXPECT_EQ(c.seconds(), -Unit::Max);
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(-Unit::Max);
  b = Unit(-1);
  c = a + b;
  EXPECT_TRUE(c.isNegativeInfinity());
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(-Unit::Max + 2);
  b = Unit(-3);
  c = a + b;
  EXPECT_TRUE(c.isNegativeInfinity());
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(-Unit::Max);
  b = Unit(-3);
  c = a + b;
  EXPECT_TRUE(c.isNegativeInfinity());
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(-Unit::Max, PicosPerSecond - 1);
  b = Unit(0, -1);
  c = a + b;
  EXPECT_TRUE(c.isNegativeInfinity());
  DUMP(a); DUMP(b); DUMP(c);

  // InfP plus.
  a = Unit(Category::InfP);
  b = Unit(Category::InfP);
  c = a + b;
  EXPECT_TRUE(c.isPositiveInfinity());
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(Category::NaN);
  b = Unit(1);
  c = a + b;
  EXPECT_TRUE(c.isNaN());
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(1);
  b = Unit(Category::NaN);
  c = a + b;
  EXPECT_TRUE(c.isNaN());
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(Unit::Max - 1);
  b = Unit(1);
  c = a + b;
  EXPECT_EQ(c.seconds(), Unit::Max);
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(Unit::Max);
  b = Unit(1);
  c = a + b;
  EXPECT_TRUE(c.isPositiveInfinity());
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(Unit::Max - 2);
  b = Unit(3);
  c = a + b;
  EXPECT_TRUE(c.isPositiveInfinity());
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(Unit::Max);
  b = Unit(3);
  c = a + b;
  EXPECT_TRUE(c.isPositiveInfinity());
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(Unit::Max, PicosPerSecond - 1);
  b = Unit(0, 1);
  c = a + b;
  EXPECT_TRUE(c.isPositiveInfinity());
  DUMP(a); DUMP(b); DUMP(c);

  // Signage.
  a = Unit(Unit::Max, 0);
  b = Unit(0, -2);
  c = a + b;
  d = c + Unit(0, 2);
  EXPECT_EQ(a, d);
  DUMP(a); DUMP(b); DUMP(c); DUMP(d);
  a = Unit(-Unit::Max, 0);
  b = Unit(0, 2);
  c = a + b;
  d = c + Unit(0, -2);
  EXPECT_EQ(a, d);
  DUMP(a); DUMP(b); DUMP(c); DUMP(d);
  a = Unit(1, 1, 2);
  b = Unit(1, 1, 2);
  c = a + b;
  EXPECT_EQ(c, Unit(3));
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(1, PicosPerSecond - 1);
  b = Unit(1, PicosPerSecond - 1);
  c = a + b;
  EXPECT_EQ(c, Unit(3, PicosPerSecond - 2));
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(-1, PicosPerSecond - 1);
  b = Unit(-1, PicosPerSecond - 1);
  c = a + b;
  EXPECT_EQ(c, Unit(-3, PicosPerSecond - 2));
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(1, 3, 4);
  b = Unit(1, 1, 4);
  c = a + b;
  EXPECT_EQ(c, Unit(3));
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(-1, 3, 4);
  b = Unit(-1, 1, 4);
  c = a + b;
  EXPECT_EQ(c, Unit(-3));
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(1, 3, 4);
  b = Unit(-1, 1, 4);
  c = a + b;
  EXPECT_EQ(c, Unit(0, 1, 2));
  DUMP(a); DUMP(b); DUMP(c);
  a = Unit(-1, 3, 4);
  b = Unit(1, 1, 4);
  c = a + b;
  EXPECT_EQ(c, Unit(0, -1, 2));
  DUMP(a); DUMP(b); DUMP(c);

  a = Unit(1) - Unit(0, 1);
  EXPECT_EQ(a, Unit(0, PicosPerSecond - 1));
  a = Unit(-1) - Unit(0, -1);
  EXPECT_EQ(a, Unit(0, -(PicosPerSecond - 1)));
  a = Unit(0) - Unit(0, 1);
  EXPECT_EQ(a, Unit(0, -1));
  a = Unit(0) - Unit(0, -1);
  EXPECT_EQ(a, Unit(0, 1));

  // Test pre/post increment/decrement.
  Unit s(5);
  const Unit t;
  auto ss = --s;
  EXPECT_EQ(s.seconds(), 4);
  EXPECT_EQ(ss.seconds(), 4);
  ss = ++s;
  EXPECT_EQ(s.seconds(), 5);
  EXPECT_EQ(ss.seconds(), 5);
  ss = s--;
  EXPECT_EQ(s.seconds(), 4);
  EXPECT_EQ(ss.seconds(), 5);
  ss = s++;
  EXPECT_EQ(s.seconds(), 5);
  EXPECT_EQ(ss.seconds(), 4);

  // Test return types.
  EXPECT_EQ(type_name<decltype(++s)>(), type_name<decltype(s)&>());
  EXPECT_EQ(type_name<decltype(s++)>(), type_name<decltype(s)>());
  EXPECT_EQ(type_name<decltype(--s)>(), type_name<decltype(s)&>());
  EXPECT_EQ(type_name<decltype(s--)>(), type_name<decltype(s)>());
  EXPECT_EQ(type_name<decltype(-s)>(), type_name<decltype(s)>());
  EXPECT_EQ(type_name<decltype(s += s)>(), type_name<decltype(s)&>());
  EXPECT_EQ(type_name<decltype(s -= s)>(), type_name<decltype(s)&>());
  EXPECT_EQ(type_name<decltype(s = s)>(), type_name<decltype(s)&>());
  EXPECT_EQ(type_name<decltype(s + s)>(), type_name<decltype(t)>());
  EXPECT_EQ(type_name<decltype(s - s)>(), type_name<decltype(t)>());

  // Test swap.
  a = Unit(1);
  b = Unit(2);
  EXPECT_EQ(a.seconds(), 1);
  EXPECT_EQ(b.seconds(), 2);
  swap(a, b);
  EXPECT_EQ(a.seconds(), 2);
  EXPECT_EQ(b.seconds(), 1);
}

TEST(ScalarMath, ChronosTest) {
  testAdd<ScalarUnit<>>();
  testAdd<Duration<>>();

  // Does not support conventional binary ops.
  //* testAdd<Moment<>>();

  Duration<> d1(1), d2(2), d3(3);
  Moment<> m1(4), m2(5), m3(6);
  d1 += d2;
  EXPECT_EQ(d1.seconds(), 3);
  d1 -= d2;
  EXPECT_EQ(d1.seconds(), 1);
  d1 = d1 + d2;
  EXPECT_EQ(d1.seconds(), 3);
  d1 = d1 - d2;
  EXPECT_EQ(d1.seconds(), 1);
  m1 = d1 + m1;
  EXPECT_EQ(m1.seconds(), 5);
  m1 = m1 + d1;
  EXPECT_EQ(m1.seconds(), 6);
  m1 = m1 - d1;
  EXPECT_EQ(m1.seconds(), 5);
  m3 -= d1;
  EXPECT_EQ(m3.seconds(), 5);
  m3 += d1;
  EXPECT_EQ(m3.seconds(), 6);
  m1 = m2 + d1;
  EXPECT_EQ(m1.seconds(), 6);
  m2 = d1 + m3;
  EXPECT_EQ(m2.seconds(), 7);
  d1 = m2 - m1;
  EXPECT_EQ(d1.seconds(), 1);
  d1 = m1 - m2;
  EXPECT_EQ(d1.seconds(), -1);

  m1 = --m2;
  EXPECT_EQ(m1.seconds(), 6);
  EXPECT_EQ(m2.seconds(), 6);
  m1 = m2--;
  EXPECT_EQ(m1.seconds(), 6);
  EXPECT_EQ(m2.seconds(), 5);
  m1 = ++m2;
  EXPECT_EQ(m1.seconds(), 6);
  EXPECT_EQ(m2.seconds(), 6);
  m1 = m2++;
  EXPECT_EQ(m1.seconds(), 6);
  EXPECT_EQ(m2.seconds(), 7);

  d1 = Duration<>(1, 1, 2);
  d1 *= 3;
  EXPECT_EQ(d1, Duration<>(4,1,2));
  d1 = Duration<>(4, 1, 2);
  d1 *= Duration<>::Max;
  EXPECT_TRUE(d1.isPositiveInfinity());
  d1 = Duration<>(0, 1, 2);
  d1 *= Duration<>::Max;
  EXPECT_TRUE(d1.isNaN());
  d1 = Duration<>(-1, 1, 2);
  d1 *= 3;
  EXPECT_EQ(d1, Duration<>(-4, 1, 2));
  d1 *= Duration<>::Max;
  EXPECT_TRUE(d1.isNegativeInfinity());
  d1 = Duration<>(0, -1, 2);
  d1 *= Duration<>::Max;
  EXPECT_TRUE(d1.isNaN());
}
