theme: Work, 1
autoscale: true
slidenumbers: true
build-lists: true
footer: IEEE 754 Decimals for C++ · Boost.Decimal · CppCon 2026

[.hide-footer]
[.slidenumbers: false]

![](img/cover.png)

^ Full-bleed title card. First words set up the next slide: "I want to
^ start with three numbers."

---

# Start with a Quiz

What is the output of this program?

```c++
#include <iomanip>
#include <iostream>

int main() {
  std::cout << std::setprecision(17) << 0.1 + 0.2;
}
```


---

# [fit] 0.30000000000000004

Why is this?

---

# Where the extra came from

The bitwise representation of `float f = 0.1f;`

![inline](img/float_0p1.png)


---

# Five does not divide two

A fraction terminates in base *b* only if every prime factor of its denominator divides *b*.[^1]

$$
\frac{1}{10} \qquad 10 = 2 \times 5 \qquad 5 \nmid 2
$$

No number of bits fixes this. The problem is not precision, but **representability**.

[^1]: Hardy & Wright, *An Introduction to the Theory of Numbers*, §9.2–9.3.

---

[.build-lists: true]

# The solution is not "more precision"

These numerical types we are about to discuss are useful when these errors are unacceptable.

- Canonical example and many users are in the financial sector
- Human-entered data and databases
- Legal and regulatory compliance

^ The domains, spoken rather than listed: billing and invoicing,
^ settlement and clearing, statutory reporting, and interchange with
^ anything that is decimal by definition — tax tables, price feeds,
^ exchange tick sizes.

---

# A brief history

- **IEEE 754-2008** specifies decimal floating point — two encodings, three interchange formats
- **ISO/IEC TR 24733** (2011) sketches a C++ binding. Never adopted into the standard.
- **C23** adds `_Decimal32` `_Decimal64` `_Decimal128` — as an *optional* feature
- **GCC** ships the types on some targets, with no accompanying standard library
- **IBM's libdfp** fills the library gap
- **Intel** ships a library with its own types

^ Every path here is either non-portable, not C++, or both.

^ If pressed on the last bullet: libdfp pairs with GCC's built-in types
^ on glibc targets — POWER, s390x, x86 Linux. Intel's library implements
^ BID entirely in software and defines its own types, so it needs no
^ compiler support at all. Both are C.


---

[.build-lists: false]

# Boost.Decimal

- Header-only, **no dependencies**, C++14
- Six types: `decimal32_t` `decimal64_t` `decimal128_t`
  and `decimal_fast32_t` `decimal_fast64_t` `decimal_fast128_t`
- `constexpr` throughout
- Its own `<cmath>` `<charconv>` `<format>`, etc.
- Boost.Math integration
- CUDA support for the types and much of the math
- Tested on x86_64, ARM64, s390x, 32-bit; emulated PPC64LE and Cortex-M

There is a JOSS paper — full citation at the end.

---

[.build-lists: false]

# Where we are going

1. Inside the bits
2. The library
3. In action
4. The trade
5. Performance
6. When to reach for it

^ The map arrives after the hook, never before it.
^
^ Twenty seconds. Do not read all six aloud — "six parts, and the first one
^ is the part nobody ever gave you." Move.

---

[.build-lists: false]

# Part 1 of 6

1. **Inside the bits**
2. The library
3. In action
4. The trade
5. Performance
6. When to reach for it

^ "This is the introduction to decimal floating point that most C++
^ programmers never got."

---

# A Binary Floating Point Refresher 

![inline](img/float_0p15625.png)

`float`: 1 + 8 + 23 = 32.

$$
(-1)^{0} \times 1.01_2 \times 2^{\,124-127} = 1.25 \times 2^{-3} = 0.15625
$$

^ Walk the fields once, left to right, so the decode is explicit:
^ sign bit 0, positive. Exponent field 01111100 is 124; subtract the
^ bias of 127 to get 2 to the minus 3. Fraction bits 01, with the
^ implied leading 1 the format never stores, read 1.01 in binary —
^ one and a quarter. 1.25 over 8 is 0.15625.
^
^ 0.15625 is exactly representable in binary AND in decimal, which is the
^ point of choosing it — this slide is about structure, not error. Nobody
^ is being cheated out of anything yet.
^
^ Forty-five seconds. Establish the model deliberately, because you are
^ about to break it.

---

# Same value with one additional field.

![inline](img/pair_0p15625.png)

$$
15625 \times 10^{-5} = 0.15625
$$

^ Let them find the orange cells. That is the whole slide.
^
^ "Same thirty-two bits. Same value. And there is a field in the bottom one
^ that has no counterpart in the top one."
^
^ Point at the annotation: the red cells gather up into 15625, and the
^ exponent says times ten to the minus five. The significand is a decimal
^ integer. That is the entire idea of BID — the decode line under the
^ strip is the whole decode. One line each way: the float needed a
^ power of two and an implied bit; this needs a decimal integer and a
^ power of ten.

---

# Why the extra field?

[.code-highlight: 1]
[.code-highlight: 1-3]
[.code-highlight: all]

```
decimal32_t holds 7 decimal digits -> max significand 9,999,999

2^23 = 8,388,608 < 9,999,999
2^24 = 16,777,216 

You need 24 bits, but sign and exponent leave you with only 23.
```

^ STEP 1 — seven digits, so the largest significand is 9,999,999.
^
^ STEP 2 — that needs 24 bits.
^
^ STEP 3 — and you have 23.
^
^ "Ten is not a power of two. The fields do not divide evenly. Everything
^ strange about this encoding is downstream of that one sentence. The
^ combination field is how you buy back the twenty-fourth bit."

---

# Four cases

[.code-highlight: 1-2]
[.code-highlight: 1-3]
[.code-highlight: 1-4]
[.code-highlight: all]

```
steer                 layout                              sig. bits
 00    s |      eeeeeeee | ttttttttttttttttttttttt            23
 01    s |      eeeeeeee | ttttttttttttttttttttttt            23
 10    s |      eeeeeeee | ttttttttttttttttttttttt            23
 11    s | 11 | eeeeeeee | [100] + ttttttttttttttttttttt      24
```

^ STEP 1 — "Zero-zero. The two steering bits concatenate onto six more for
^ eight bits of exponent, and 23 bits of significand. Exactly like float."
^
^ STEP 2 — "Zero-one. Same."
^
^ STEP 3 — "One-zero. Same." — By now somebody is wondering why you are
^ belaboring this. Let the pause run slightly too long.
^
^ STEP 4 — "One-one. Different." Exponent moves to its own eight bits, and
^ the significand gets an implied leading 0b100 plus 21 stored. Twenty-four.
^
^ "Three quarters of the encoding space is float-shaped. The last quarter
^ is where the seventh digit lives."
^
^ Two things for Q&A, not the stage: the top of the 11 space is carved
^ out for non-finites — combination 11110 is infinity, 11111 NaN, one
^ more bit picks signaling. And the masks for everything in this table
^ sit near the top of decimal32_t.hpp, reading exactly like the table;
^ anyone who wants to source-dive can start there.

---

# Watch the fields move for adjacent integers

![inline](img/boundary.png)

^ This is the slide the last three earned. Say almost nothing.
^
^ "Add one to eight million three hundred eighty-eight thousand six hundred
^ and seven. The exponent field grows by two cells. The significand shrinks
^ by two and picks up an implied one-zero-zero. Same value plus one. Total
^ structural reorganization."
^
^ If you want a laugh: "if you were hoping the bit pattern was monotonic in
^ the value, this is where that hope goes."

---

# `0.1`, both ways

![inline](img/pair_0p1.png)

^ Deliberate callback — they saw the top strip eight minutes ago and have
^ been carrying it since.
^
^ "Same width. Thirty-two bits on both sides. The bottom one contains the
^ number I asked for, and it contains it exactly. One times ten to the
^ minus one."
^
^ Bias is 101, so a biased exponent of 100 is q = -1. Have it ready.

---

# BID and DPD

IEEE 754 specifies two encodings:

- **BID**: Binary Integer Significand. Intended for software implementations.
- **DPD**: Densely Packed Decimal. Intended for hardware implementations.

Boost.Decimal uses **BID**, and can convert either way:

```c++
constexpr std::uint32_t to_bid_d32(decimal32_t val)      noexcept;
constexpr decimal32_t   from_bid_d32(std::uint32_t bits) noexcept;
constexpr std::uint32_t to_dpd_d32(decimal32_t val)      noexcept;
constexpr decimal32_t   from_dpd_d32(std::uint32_t bits) noexcept;
```

^ Plus generic to_bid/to_dpd/from_bid<T>/from_dpd<T>, which is what every
^ strip in this section actually called.
^
^ Be precise if asked about hardware: the conversions are encoding
^ interchange. The library is not tested against hardware decimal units,
^ because they are not readily accessible to test against — s390x is in
^ CI for endianness, not for its DFU.

---

# BID vs DPD for our adjacent integers

```
                          BID           DPD
decimal32_t{8388607}  0x32FFFFFF    0x6A573B07
decimal32_t{8388608}  0x6CA00000    0x6A573B08
```

**BID** rebuilds the word.

**DPD** increments by one.

^ Hex is right here, because the delta IS the point.
^
^ "DPD packs three decimal digits into every ten-bit declet, so 607 and 608
^ live in the low bits and nothing else moves. BID treats the significand as
^ one binary integer, so changing the integer changes the encoding."
^
^ "Digit locality is what you want when digits are wired into silicon. A
^ single binary integer is what you want in software."

---

# [fit] Now the part with no

# [fit] binary analogue

^ Pacing beat. This concept is what makes the rest of the talk cohere, and
^ you call back to it four more times.

---

# One value. Seven encodings.

> The set of representations a floating-point number maps to is called the floating-point number's cohort.
-- IEEE 754-2008

```
decimal32_t{      3,  2}   ->   0x33800003
decimal32_t{     30,  1}   ->   0x3300001E
decimal32_t{    300,  0}   ->   0x3280012C
decimal32_t{3000000, -4}   ->   0x30ADC6C0
```

- All compare equal with `operator==`
- All **hash equal**
- None are bitwise equal

^ Seven members in 300's cohort, because seven digits give 3 exactly
^ seven stops: 3 times ten squared out to 3000000 times ten to the
^ minus four. Four shown, for space.
^
^ Binary has exactly one way to write each value, so nothing in
^ anyone's experience prepares them for this.
^
^ "operator== says these are the same number. std::hash agrees, because it
^ has to. memcmp does not."
^
^ Do not resolve it — Part 2 is where charconv pays this off.

---

# Normalizing

A value is **normalized** when the cohort effect is removed by appending zeros to the significand and reducing the exponent until it carries the type's full precision:

$$
3 \times 10^{2} \quad \longrightarrow \quad 3000000 \times 10^{-4}
$$

Every arithmetic operation and every comparison has to handle the effects of cohorts.

This is among the most expensive parts of these operations.

---

# [fit] Rounding

^ Short divider, or run the next two straight — your call on the clock.

---

[.build-lists: true]

# Binary Floating Point Rounding

In `<cfenv>` we have the following four macros useable with `std::fesetround()`:

1. `FE_DOWNWARD` - Rounding towards negative infinity 
2. `FE_TONEAREST` - Rounding towards nearest representable value
3. `FE_TOWARDZERO`- Rounding towards zero
4. `FE_UPWARD` - Rounding towards positive infinity 

---

# The Five Decimal Rounding Modes

All modes are in `enum class rounding_mode` and useable with `boost::decimal::fesetround()`:

1. `fe_dec_downward`
2. `fe_dec_to_nearest` - To nearest, ties to even
3. `fe_dec_to_nearest_from_zero`
4. `fe_dec_toward_zero` - The opposite of 3
5. `fe_dec_upward`

Default is #2, per IEEE 754 4.3.3 and is called "banker's rounding", and is available also as *`fe_dec_default`*

---

# Example of Rounding Mode

```c++
#include <boost/decimal.hpp>
#include <iostream>

int main() {
    using namespace boost::decimal::literals;
    using boost::decimal::decimal32_t;

    boost::decimal::fesetround(boost::decimal::rounding_mode::fe_dec_upward); // NOT THREAD-SAFE

    const decimal32_t lhs {"5e+50"_DF};
    const decimal32_t rhs {"4e+40"_DF};
    const decimal32_t sum {lhs + rhs};

    std::cout << "5e50 + 4e40 = " << sum << std::end;
}
```
Output: `5e50 + 4e40 = 5.000001e+50`

^ Even though the difference in order of magnitude is greater than the precision of the type, any addition in this mode will result in at least a one ULP difference

---

# Compile Time Rounding

Very similar to the run-time `enum class`, but we now have the following macros:

1. `BOOST_DECIMAL_FE_DEC_DOWNWARD`
2. `BOOST_DECIMAL_FE_DEC_TO_NEAREST`
3. `BOOST_DECIMAL_FE_DEC_TO_NEAREST_FROM_ZERO`
4. `BOOST_DECIMAL_FE_DEC_TOWARD_ZERO`
5. `BOOST_DECIMAL_FE_DEC_UPWARD`

^ Each of these are the same as their runtime counterparts. Must be defined prior to any decimal header

---

# Changing the Compile Time Rounding Mode

```c++
#define BOOST_DECIMAL_FE_DEC_DOWNWARD   // before ANY decimal header

#include <boost/decimal/decimal32_t.hpp>
#include <boost/decimal/literals.hpp>

using namespace boost::decimal::literals;
using boost::decimal::decimal32_t;

constexpr decimal32_t lhs {"5e+50"_DF};
constexpr decimal32_t rhs {"4e+40"_DF};
constexpr decimal32_t res {lhs - rhs};

static_assert(res == "4.999999e+50"_DF, "Incorrectly rounded result");
```

^ Three points, one slide. Do all three.
^
^ 1. Magnitudes differ by ten orders of magnitude — further apart than the
^    precision of the type. In a directed mode you still move a full ULP.
^    Changing the rounding mode changes your answers.
^
^ 2. The macro works on every compiler, and being read-only it is thread
^    safe. There is a runtime fesetround too, which needs consteval
^    detection and is NOT thread safe. One sentence each.
^
^ 3. That is a static_assert on a rounded decimal subtraction. Seed for
^    Part 4 — we got constexpr, and we paid for it.

---

# Three things to carry forward

1. Each value has many encodings. `==` is not `memcmp`.
2. Normalizing is per-operation work.
3. Five rounding modes available instead of four.

^ Sixty seconds. End of Part 1, and you should be at roughly 17:00.

---

[.build-lists: false]

# Part 2 of 6

1. Inside the bits
2. **The library**
3. In action
4. The trade
5. Performance
6. When to reach for it

^ "You now know more about the encoding than you will ever need in order to
^ use it. This part is the part you do need."
^
^ Ten seconds. Do not linger on dividers.

---

# One Include

```c++
#include <boost/decimal.hpp>
```

Header-only. No dependencies. C++14.

```c++
import boost.decimal; // C++20 module
```

- Available individually through vcpkg and conan, or with Boost in most package managers
- Build with b2 or CMake

^ Fifteen seconds. Nobody came for the build system, but somebody will ask,
^ so put it on screen once and never mention it again.
^
^ The umbrella pulls in everything that has no external dependency. There is
^ exactly one exception and you will name it in eight minutes.

---

# Six types

Three IEEE 754 compliant types:
```
                     bytes   digits10       exponent range
decimal32_t              4          7        -95 ..    +96
decimal64_t              8         16       -383 ..   +384
decimal128_t            16         34      -6143 ..  +6144
```
Three performance-focused types:
```
decimal_fast32_t         8          7        -95 ..    +96
decimal_fast64_t        16         16       -383 ..   +384
decimal_fast128_t       32         34      -6143 ..  +6144
```

The performance costs exactly double the size.

^ No build. Let them read it — this is the one table in the talk that
^ rewards silence.
^
^ Every number here is numeric_limits, not the docs. Pulled with a program.
^
^ "The fast types are not a different numeric model. They are the same
^ numbers in a roomier box."

---

# What the extra bytes buy

[.code-highlight: 1-4]
[.code-highlight: all]

```c++
class decimal32_t final {
    std::uint32_t bits_;            // IEEE 754 BID
};

class decimal_fast32_t final {
    std::uint32_t significand_;     // stored decoded and normalized
    std::uint8_t  exponent_;
    bool          sign_;
};
```

^ For decimal32_t every operation starts with decoding the value into
^ significand, exponent, and sign. With the fast types we skip this
^ entirely and store the value always normalized.

---

# Which one to use?

**Does the bit pattern matter?**

- **Yes**: a file, a socket, a column, another language, another vendor
  → `decimal32_t` `decimal64_t` `decimal128_t`
- **No**: computed, compared, and discarded
  → `decimal_fast32_t` `decimal_fast64_t` `decimal_fast128_t`

^ Yes is effectively when using it as an interchange format
^ No is using it for your own computations

---

# Construction

[.code-highlight: 1]
[.code-highlight: 2]
[.code-highlight: 3]
[.code-highlight: 4]
[.code-highlight: 5]

```c++
decimal32_t a {1, 1};                                  // 1e1
decimal32_t b {-2, -1};                                // -0.2
decimal32_t c {2U, -1, construction_sign::negative};   // -0.2
decimal32_t d {"4.3e-02"};                             // string or string_view
auto        e {"3.14159265358979"_DF};                 // literal, rounds to fit
```

^ Thirty seconds. The design point worth saying out loud: a signed
^ coefficient or an explicit sign, never both — because what would the
^ sign of {-3, 0, negative} be? The API refuses to be asked.
^
^ Literals live in namespace boost::decimal::literals, like std::literals.
^ _DF _DD _DL, and add an f for the fast types.

---

# It promotes like you expect

```c++
decimal32_t{}       + decimal64_t{}        ->  decimal64_t
decimal64_t{}       * 2                    ->  decimal64_t
decimal64_t{}       + decimal_fast32_t{}   ->  decimal64_t
decimal_fast128_t{} - decimal128_t{}       ->  decimal_fast128_t
```

Wider precision wins. On a tie, **fast wins**.

^ All four of those are static_asserts in my notes, not claims from the
^ documentation. Twenty seconds.
^
^ Mixed comparison works across all six types and against integers, and it
^ is exact even when the value is not representable in the narrower type —
^ which is exactly the trap you fall into if you cast by hand first.

---

# What it will not do quietly

```c++
decimal64_t d {3.14};       // explicit only, and discouraged
int n = d;                  // ill-formed
```

^ This is a seed. You spend that macro on the Boost.Math slide in about
^ four minutes, and because you planted it here it lands as a consequence
^ rather than an apology.
^
^ Twenty-five seconds.

---

[.build-lists: false]

# What comes in the box

```
<cmath>        over a hundred functions, all constexpr
<charconv>     to_chars / from_chars, plus two decimal-only formats
<format>       and {fmt}
<cstdlib>      strtod32 / strtod64 / strtod128
<cfenv>        rounding modes
<cfloat>       evaluation modes
<functional>   std::hash, all six types
<limits>       numeric_limits, all six types
<string>       to_string, stod32 ...
<iostream>     operator<< and operator>>
<numbers>      pi, e, ln2, ... per type
```

Everything lives in `boost::decimal`, except literals, which live in `boost::decimal::literals`.

---

# Generic code: the `std::swap` two-step

```c++
template <typename T>
void sin_identity(T val)
{
    using std::sin;
    using boost::decimal::sin;

    sin(val);           // unqualified. float, double, decimal128_t — all fine.
}
```

^ example/adl.cpp is the runnable version.
^
^ The idiom is the one everybody already knows from swap: pull both
^ overload sets in with using-declarations, call unqualified, let ADL
^ pick. Boost.Decimal is not allowed to overload std::sin, so its sin
^ lives in boost::decimal — same reason, same cure.

---

# All of it is `constexpr`

```c++
constexpr decimal64_t two  {2};
constexpr decimal64_t root {sqrt(two)};

static_assert(root == "1.414213562373095"_DD, "");
```

`std::sqrt` is not `constexpr` until C++26.

^ Compiled at -std=c++14 to make sure. Fifteen seconds, then land the last
^ line and stop talking.
^
^ Part 4 is where you explain what this cost — the exception flags. Do not
^ trail it here, just let them enjoy it.

---

# Two ways to decompose a value

Three spellings of `300`, taken apart two ways:

```
decimal32_t{300}          decompose  ->  sig 300,      exp  0
                          frexp10    ->  sig 3000000,  exp -4

decimal32_t{3, 2}         decompose  ->  sig 3,        exp  2
                          frexp10    ->  sig 3000000,  exp -4

decimal_fast32_t{3, 2}    decompose  ->  sig 3000000,  exp -4
                          frexp10    ->  sig 3000000,  exp -4
```

- `decompose` retains the cohort
- `frexp10` normalizes it away
- The fast types return the same for both since they store normalized

^ example/decompose_frexp10_normalize.cpp, verbatim.

^ These functions both live in the <cmath> header

---

# [fit] A deeper look into `<charconv>`

^ We will not look at our expanded version of <charconv> as being able to process base-10 values is one of the key selling points of the library 

---

# `chars_format` options

```c++
enum class chars_format : unsigned
{
    scientific,
    fixed,
    hex,
    cohort_preserving_scientific,     // not in <charconv>
    cohort_preserving_fixed,          // not in <charconv>
    general = fixed | scientific
};
```

^ Scientific, fixed, hex and general are the same as you are used to

^ Cohort preserving scientific and fixed will reject values that require any rounding

---

# The round trip

[.code-highlight: 1-7]
[.code-highlight: all]

`<charconv>` using `chars_format::cohort_preserving_scientific`

```
"3e+02"         ->  0x33800003  ->  "3e+02"
"3.0e+02"       ->  0x3300001E  ->  "3.0e+02"
"3.00e+02"      ->  0x3280012C  ->  "3.00e+02"
"3.000e+02"     ->  0x32000BB8  ->  "3.000e+02"
"3.0000e+02"    ->  0x31807530  ->  "3.0000e+02"
"3.00000e+02"   ->  0x310493E0  ->  "3.00000e+02"
"3.000000e+02"  ->  0x30ADC6C0  ->  "3.000000e+02"
```

^ THE payoff slide for the whole cohort thread. Earn it with silence.
^
^ STEP 1 — the round trips. "Every one of those is `==` to every other one."
^
^ STEP 2 — the format name. "One enumerator. That is the entire feature."
^
^ "If you write 3.00 to a ledger, you get 3.00 back out of the ledger. Not
^ 3, and not 3.000000."

---

# Sizing buffers

```c++
template <typename DecimalType, int Precision = std::numeric_limits<DecimalType>::max_digits10>
class formatting_limits
{
public:
    static constexpr std::size_t scientific_format_max_chars;
    static constexpr std::size_t fixed_format_max_chars;
    static constexpr std::size_t hex_format_max_chars;
    static constexpr std::size_t cohort_preserving_scientific_max_chars;
    static constexpr std::size_t cohort_preserving_fixed_max_chars;
    static constexpr std::size_t general_format_max_chars;
    static constexpr std::size_t max_chars;
};
```

This simplifies correct buffer sizing to:

```c++
char buf[formatting_limits<decimal64_t>::scientific_format_max_chars];
```

^ Max chars is the absolute maximum of all listed formats

---

# [fit] Integration with other libraries

---

# Two ways to format

```
                    needs                              standard
<fmt/format.h>      {fmt} 11 or newer                  C++14
<format>            GCC 13 · Clang 18 · MSVC 19.40     C++20
```

```c++
fmt::format("{:*>+12.2e}", val);      // [***+3.14e+00]
std::format("{:*>+12.2e}", val);      // [***+3.14e+00]
```


^ Thirty seconds.
^
^ Be accurate about what this is: these are adapters, and we wrote them.
^ format.hpp and fmt_format.hpp are a thousand lines of formatter
^ specialization between them, plus another hundred and thirty for the
^ std::hash specializations. The user does not write an adapter. We did.
^
^ THIS is the one header the umbrella does not pull in: fmt_format.hpp,
^ because {fmt} is an external and possibly-compiled dependency. Include it
^ yourself. That is the exception you promised eight minutes ago.
^
^ The {fmt} floor is 11, because detection keys on <fmt/base.h>. Verified
^ against 9.1, 10.2 and 11.1 — the first two are not detected.

---

# One extra formatting specifier

```
g G    general
e E    scientific
f      fixed
x X    hex
a A    cohort preserving scientific
```

```c++
fmt::format("{}",   d);     // 300
fmt::format("{:a}", d);     // 3.00e+02
```

^ Alignment, fill, width, sign, precision and the L locale modifier all
^ behave exactly as they do for built-in floating point. 

---

# Boost.Math

Bollinger bands over a year of AAPL closes.

```c++
#include <boost/math/statistics/univariate_statistics.hpp>

const decimal64_t mean   = boost::math::statistics::mean(closes);
const decimal64_t median = boost::math::statistics::median(closes);
const decimal64_t var    = boost::math::statistics::variance(closes);
const decimal64_t sigma  = boost::decimal::sqrt(var);
```

```
  Mean Closing Price: $207.20
  Standard Deviation: $25.45
Upper Bollinger Band: $258.11
Lower Bollinger Band: $156.30
```

^ Forty seconds.
^
^ "Boost.Math's statistics are generic over Real, and decimal64_t is a
^ Real." This one really is just the include — unlike <format> and {fmt},
^ where we wrote the formatter, nobody wrote anything to make this work.
^
^ The honest footnote, and say it rather than let them find it in the
^ example file later: statistics.cpp needs
^ BOOST_DECIMAL_ALLOW_IMPLICIT_INTEGER_CONVERSIONS plus -Wfloat-equal and
^ -Wsign-conversion suppressed. Boost.Math assigns integer literals
^ straight into Real and our constructors are explicit. Verified both
^ ways: without the macro, single_pass.hpp fails at the same two sites
^ against Boost 1.83's Math AND against Math develop (2026-08-05).
^
^ closes is a std::vector<decimal64_t> parsed with from_chars.

---

# On the GPU

```c++
#define BOOST_DECIMAL_ENABLE_CUDA
#include <boost/decimal/decimal64_t.hpp>

__global__ void add(const decimal64_t* a, const decimal64_t* b,
                    decimal64_t* out, int n)
{
    const int i {blockDim.x * blockIdx.x + threadIdx.x};
    if (i < n) { out[i] = a[i] + b[i]; }
}
```

Device results compare **exactly equal** to the host loop.

^ Twenty seconds, and do not oversell it — the types and much of the math
^ are device-capable, not all of it. Functions carrying HOST_DEVICE or
^ CUDA_CONSTEXPR are the ones that run on device.

---

# Debugging with LLDB and GDB

![inline](img/debugger_pair.png)

^ The debugger understands and represents cohorts

^ We recently added support for NATVIS

---

[.build-lists: false]

# Part 3 of 6

1. Inside the bits
2. The library
3. **In action**
4. The trade
5. Performance
6. When to reach for it

^ Nine examples. The two forensics lean on the price feed; everything
^ else stands alone, so any of them can go if the clock is against you.
^
^ Seven of the nine are files in example/ in the repository; the two
^ forensics run from the probe shipped alongside this deck. Say that
^ once, here, and do not repeat it per slide.

---

# Adding `0.1` a thousand times

```c++
constexpr decimal32_t decimal_one_tenth {"0.1"};
constexpr float       float_one_tenth   {0.1F};

for (int i {}; i < 1000; ++i)
{
    decimal_value += decimal_one_tenth;
    float_value   += float_one_tenth;
}
```

```
Decimal Result: 100
  Float Result: 99.999
```

^ example/addition.cpp
^
^ Thirty seconds. Shortest complete demonstration in the library, and the
^ one people repeat to their colleagues afterwards.
^
^ The decimal constant comes from a string, not from 0.1F. Going through
^ the float literal would import the binary error before the loop started.

---

# Parsing a price feed

252 daily opening prices, one CSV, parsed twice.

```c++
decimal32_t price;
from_chars(token, price);                       // decimal column

const float f {std::stof(token)};               // float column
```

```
Number of data points: 252
    Sum from MS Excel: 52151.99
Sum using decimal32_t: 52151.99
      Sum using float: 52151.96
```

^ example/numerical_parsing.cpp
^
^ Say it plainly: "this is the program from the first slide." They have
^ been carrying these numbers since the top of the hour; here is the
^ loop that made them.
^
^ Do not answer parse-versus-sum from the stage yet. The next two slides
^ take the three cents apart properly.

---

# The three cents, itemized

[.code-highlight: 1-2]
[.code-highlight: 1-3]
[.code-highlight: 1-4]
[.code-highlight: all]

```
parse as       accumulate in     sum
float          float             52151.964844
float          double            52151.989944
double         double            52151.990000

decimal32_t    decimal32_t       52151.99
```

**The parses cost six thousandths of a cent. The 252 additions cost the rest.**

^ deck_probes.cpp, shipped alongside this deck; numbers at 0429456.
^
^ Step the rows like the opening slide — these are the same three
^ numbers, taken apart.
^
^ Row 2 isolates the parse: float parse, double accumulate, and the
^ damage is six thousandths of a cent. Row 1 turns the accumulation to
^ float and loses two and a half cents more. Partial sums up near
^ fifty-two thousand land on a grid of 1/256 — the spacing of floats
^ between 2^15 and 2^16 — and 252 landings drift.
^
^ Say the hammer out loud: a float significand counts integers to
^ 16,777,216; decimal32_t stops at 9,999,999. The type with the SMALLER
^ significand is the exact one, because every partial sum here is a
^ seven-digit decimal value. Representability, not precision — the same
^ sentence as the top of the hour. (Do not say "more digits": float's
^ digits10 is 6 to our 7. Integer range is the honest comparison.)
^
^ Kahan defusal, when it comes: compensated float summation lands on
^ 52151.988281. It repairs the drift, cannot repair the parse, and still
^ cannot hold 52151.99. Right answer to a different question.
^
^ Row 3, for whoever whispers "so use double": at full precision it is
^ 52151.989999999991, and the display rounds it home. The fee run at the
^ end of this part is where that stops being good enough.

---

# The sum depends on the sort

Same 252 values. Three iteration orders.

```
                  float           decimal32_t
file order        52151.964844    52151.99
ascending         52151.984375    52151.99
descending        52151.992188    52151.99
```

1,000 random shuffles: **19 distinct float sums**. One decimal sum.

^ Same probe. The shuffles are mt19937 seed 42, each permutation fed to
^ both types.
^
^ Descending happens to land on 52151.99 at two decimals — the bug
^ appears and disappears with the sort. Your nightly job sums in date
^ order; the auditor sorts by amount; std::reduce with a parallel policy
^ is entitled to any order it likes. Same column, and the cent is a
^ function of iteration order.
^
^ Why decimal32_t holds still: every partial sum of this file is a
^ two-decimal value under a hundred thousand — seven significant digits —
^ so all 252 partials are exact, and the probe checks each one against
^ integer cents. Exact partials cannot care about order. Say the durable
^ rule too: decimal floating point is NOT associative in general;
^ two-decimal money stays exact for as long as the running total fits
^ the type, and 52151.99 fits decimal32_t with nothing to spare.
^
^ The 19 float answers span 52151.949219 to 52152.023438 — a 7.4-cent
^ spread across reorderings of the same column.

---

# Rounding to the cent

```c++
const decimal64_t cent {"0.01"};

quantize(decimal64_t{"123.456789"}, cent);      // 123.46
quantize(decimal64_t{"1.015"},      cent);      // 1.02
quantize(decimal64_t{"0.145"},      cent);      // 0.14
```

^ Thirty seconds.
^
^ This is the IEEE 754 quantize operation: it gives the result the quantum
^ exponent of its second argument. You are not multiplying by a hundred and
^ dividing by a hundred — you are telling the value what its exponent is.
^
^ Rounding follows the active mode, so these are ties to even. 1.015 goes
^ up to 1.02 and 0.145 goes down to 0.14, and both of those are correct.

---

# The same question in `double`

```
value      std::printf("%.2f")   round(x*100)/100   quantize(x, cent)
1.005            1.00                  1.00               1.00
1.015            1.01                  1.01               1.02
1.025            1.02                  1.02               1.02
2.675            2.67                  2.68               2.68
8.835            8.84                  8.84               8.84
0.145            0.14                  0.14               0.14
1.115            1.11                  1.12               1.12
```

^ Slow down here. Two separate findings in one table and they are worth
^ separating out loud.
^
^ First: 2.675 and 1.115. The two double spellings disagree with each
^ other. Same program, same value, same intent. Neither function is buggy —
^ 2.675 is not in the format, so there is no tie for a tie-breaker to
^ break, and which side you land on depends on the arithmetic you did to
^ get there.
^
^ Second: 1.015. Every double answer is 1.01 and decimal says 1.02. Ties
^ to even says 1.02. Double is not breaking the tie differently, it never
^ had one — the stored value is 1.0149999999999999.
^
^ 0.145 is in the table on purpose, so that nobody leaves thinking decimal
^ just always rounds up.

---

# Reconciling a fee run

```c++
for (const auto price : closes)                 // 252 closes, 10 bps
{
    const decimal64_t exact  {price * rate};
    const decimal64_t billed {quantize(exact, cent)};

    exact_total    += exact;
    billed_total   += billed;
    residual_total += (exact - billed);
}
```

```
decimal64_t   billed + residual   52.21589              == exact
double        billed + residual   52.215889999999881    != exact
```

^ Forty seconds.
^
^ "What you billed plus what you dropped has to equal what you computed.
^ Not approximately."
^
^ Both sides bill 52.22, so the invoice is right either way. It is the
^ books that do not close.
^
^ The double side uses nearbyint, which is ties to even, the same mode —
^ this gap is not a rounding policy disagreement, it is the format. If you
^ reach for std::round instead you get ties away from zero and a billed
^ total of 52.24, which is a different problem and also worth knowing.

---

# Reading and writing a file

```c++
const std::uint32_t word {to_bid(value)};
out.write(reinterpret_cast<const char*>(&word), sizeof(word));

const decimal32_t recovered {from_bid<decimal32_t>(word)};
```

```
 Current value: 0.000506           Value as bytes: 2dcd4c57
 Current value: -3.808117e+34      Value as bytes: c0ba1b75
 Current value: -1.656579e-12      Value as bytes: a9994703

Successfully recovered all values from file
```

^ example/to_from_file.cpp
^
^ Thirty seconds. Fixed stride, no parser, and the cohort rides along in
^ the word — recovered values are bitwise equal, not merely equal.
^
^ Do not claim a size win. For short values the text file is smaller. What
^ you are buying is fixed width and no round-trip through a parser.

---

# When the value does not fit

```c++
decimal32_t {100,  10000};                              // inf
decimal32_t {100, -10000};                              // 0
decimal32_t {std::numeric_limits<std::uint64_t>::max()} // 1.844674e+19

static_cast<std::uint32_t>(quiet_NaN);                  // UINT32_MAX
static_cast<std::uint64_t>(infinity);                   // UINT64_MAX

decimal32_t {"Junk_String"};                            // throws
```

^ example/basic_construction.cpp and example/integral_conversions.cpp
^
^ Forty seconds, and it is the least glamorous slide in the talk. Do it
^ anyway — this is the first thing anyone evaluating the library will
^ actually poke at.
^
^ Overflow and underflow behave like binary floating point. Integers have
^ no infinity and no NaN, so both saturate to max. The string constructor
^ throws, or returns a quiet NaN if exceptions are off, and the library
^ detects which environment it is in.

---

[.build-lists: false]

# Part 4 of 6

1. Inside the bits
2. The library
3. In action
4. **The trade**
5. Performance
6. When to reach for it

^ Placeholder so the glide path stays wired end to end. Part 4 content
^ follows.
