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

# The Reason? Five does not divide two

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

---

# A brief history and availability

- **IEEE 754-2008** specifies decimal floating point with two encodings and three interchange formats
- **ISO/IEC TR 24733** (2011) sketches a C++ binding. Never adopted into the standard.
- **N3871** (2014) second attempt at adding decimal types to the C++ standard
- **C23** adds `_Decimal32` `_Decimal64` `_Decimal128` as an *optional* feature
- **GCC** libstdc++ ships these types on selected targets (`<decimal/decimal>`), without an accompanying standard library
- **IBM's libdfp** fills the library gap
- **Intel** ships a library with its own types

^ There's an active P-paper for rebasing <cmath> on C23 <math.h>, but it specifically excludes Decimal types

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
- Tested on `x86_64`, `x86_32`, ARM64, ARM32, s390x; emulated PPC64LE and Cortex-M
- There is a JOSS paper with full citation at the end of this talk

---

[.build-lists: false]

# Part 1 of 5

1. **Inside the bits**
2. The Types
3. The Standard Library
4. Performance
5. When to reach for it

^ "This is the introduction to decimal floating point that most C++ programmers never got."

---

# A Binary Floating Point Refresher 

![inline](img/float_0p15625.png)

`float` bits: 1 sign + 8 exponent + 23 significand = 32.

$$
(-1)^{0} \times 2^{\,124-127} \times 1.01_2 = 1.25 \times 2^{-3} = 1.25 \div 8 = 0.15625
$$

^ 1. sign bit 0, positive. 
2. Exponent field 01111100 is 124; subtract the
bias of 127 to get 2 to the minus 3. 
3. Fraction bits 01, with the implied leading 1, read 1.01 in binary give us 1.25. 
4. 1.25 over 8 is 0.15625.
5. 0.15625 is exactly representable in binary AND in decimal, which is why it was chosen

---

# Same value with one additional field.

![inline](img/pair_0p15625.png)

$$
15625 \times 10^{-5} = 0.15625
$$

^ Now we've add the combination field. The way that decimal32_t represents this value is 15625 x 10^-5. 
You can find 15625 in the lower half of the significand, and then we will talk about the rest

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

[.build-lists: false]

# Part 2 of 5

1. Inside the bits
2. **The Types**
3. The Standard Library
4. Performance
5. When to reach for it

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

# Part 3 of 5

1. Inside the bits
2. The Types
3. **The Standard Library**
4. Performance
5. When to reach for it

^ "You now know more about the encoding than you will ever need in order to
^ use it. This part is the part you do need."
^
^ Ten seconds. Do not linger on dividers.

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

# Boost.Math Integration

Bollinger bands over a year of AAPL closes.

```c++
#include <boost/math/statistics/univariate_statistics.hpp>
#include <boost/decimal.hpp>

std::vector<decimal64_t> closes;

// Import closing data from a data source of AAPL 

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

^ In prior releases you needed BOOST_DECIMAL_ALLOW_IMPLICIT_INTEGER_CONVERSIONS.

^ This is from the example/ directory in the repo

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

Device results compare to the same loop run on the host

---

# Debugging Pretty Printers

![inline](img/debugger_pair.png)

^ The debugger understands and represents cohorts

^ Support is available for GDB, LLDB and NATVIS

---

[.build-lists: false]

# Part 4 of 5

1. Inside the bits
2. The Types
3. The Standard Library
4. **Performance**
5. When to reach for it

---

# Benchmarks

Complete Benchmarks are available for the following systems:

1. x86_64 Linux
2. x86_32 Linux
3. x86_64 Windows
4. ARM64 Windows
5. ARM64 MacOS

GCC `_DecimalXX` and Intel `BID_UINTXX` are included where available for completeness

^ Linux uses both GCC and the new Clang-Based Intel Compiler

^ Windows uses MSVC

^ macOS uses homebrew Clang 22 on an M4 Max MacBook Pro

^ Methodology: 20-million-element random vectors, element-wise operations, five repetitions. Every number that follows is derived from the runtime columns of the published benchmark tables.

<!-- perf figures: img/perf_*.png generated by make_perf_charts.py from
     perf_data.py; data transcribed from
     https://www.boost.org/doc/libs/develop/libs/decimal/doc/html/benchmarks.html
     (develop docs, fetched 2026-08-27), cross-verified against the published
     ratio-to-double column (474 entries). Normalization: vendor charts vs
     same-width decimalN_t; ARM64 and charconv charts vs hardware double.
     Palette: stand-in hexes in perf_data.py pending the bit-strip values. -->

---

# 32-bit Types

![inline](img/perf_x64linux_gcc_32.png)


^ Everything is runtime relative to decimal32_t = 1.00 on x86_64 Linux with GCC 14.

^ Baseline context the chart hides: decimal32_t itself runs roughly 19–39× a hardware double on this platform depending on the operation. Software against hardware — decimal will be slower, and we say so up front.

^ Honesty notes for Q&A: the GCC and Intel numbers come from very close but not identical benchmark routines, written in C rather than C++ (per the docs). And under Intel's own compiler, their 32-bit library wins several operations against decimal32_t — multiplication even against decimal_fast32_t. The appendix has that chart.

---

# 64-bit Types

![inline](img/perf_x64linux_gcc_64.png)

One honest loss: GCC `_Decimal64` compares faster than we do

^ _Decimal64 takes comparisons at 0.56× and effectively ties multiplication at 1.00×; the Boost types hold addition, subtraction, and division. Intel's BID_UINT64 comparisons are the outlier at 5.87× — it sets the shared scale.

---

# 128-bit Types

![inline](img/perf_x64linux_gcc_128.png)

The closest race of the three widths

^ GCC _Decimal128 genuinely takes multiplication (0.68× of decimal128_t, with decimal_fast128_t at 1.06×). Intel BID_UINT128 beats decimal128_t on addition (0.89×) but not decimal_fast128_t (0.38×). Everything else goes to the Boost types — including the 0.09× comparison number for decimal_fast128_t, consistent with the fast layout skipping the decode step entirely.

---

# Where decimal beats the hardware

![inline](img/perf_charconv_x64linux.png)

- `<charconv>` is software for every type so the hardware advantage disappears

^ This is also the one case where the fast types are not faster, they have to perform the normalization arithmetic that the regular types do not have to do.

^ Boost.Charconv is about 15,000 lines for the full implementation whereas this is under 2,000 in decimal

---

# Part 5 of 5

1. Inside the bits
2. The Types
3. The Standard Library
4. Performance
5. **When to reach for it**

---

# Use Cases Revisited

At the top of the presentation we suggested:

- Canonical example and many users are in the financial sector
- Human-entered data and databases
- Legal and regulatory compliance

---

# Current Clients Applications

- A trading firm, because their home-brew fixed point could not represent the smallest divisible unit of a Bitcoin (A Satoshi = 1/100,000,000 of a Bitcoin)
- Several quant firms, who use `<charconv>` extensively
- A database firm, who needed `decimal128_t` to be run on ARM
- Boost.MySQL for the DECIMAL data type (in conjunction with Boost.Multiprecision)
  
^ As Boost is typically consumed from package managers it is generally quite difficult to know who your customers are. 
These ones have specifically engaged throughout the development process both publicly or privately

---

# Takeaways

- Decimal Floating Point types can provide a new way to handle data and computations, but there is a performance cost
- Boost.Decimal is portable, performant, and available everywhere.

---

# Questions and Citation


Borland and Kormanyos, (2026). Boost.Decimal: A C++14 Library for Decimal Floating Point Arithmetic. Journal of Open Source Software, 11(124), 10345, https://doi.org/10.21105/joss.10345

There is also a CITATION.cff file in the repo


---

# Appendix

^ Backup material — full platform-by-platform benchmark coverage. Not part of the timed talk.

---

# x86_64 Linux · Intel oneAPI — 32 bits

![inline](img/perf_x64linux_intel_32.png)

^ icpx compiles the Boost.Decimal harness, icx compiles Intel's libbid — same 20-million-element methodology. This is the config where Intel's 32-bit library wins addition, multiplication, and division against decimal32_t; decimal_fast32_t still leads everywhere except multiplication.

---

# x86_64 Linux · Intel oneAPI — 64 bits

![inline](img/perf_x64linux_intel_64.png)

---

# x86_64 Linux · Intel oneAPI — 128 bits

![inline](img/perf_x64linux_intel_128.png)

---

# x86_32 Linux · GCC 14 (-m32) — 32 bits

![inline](img/perf_x32linux_gcc_32.png)

^ Same runner as x86_64 Linux, built -m32. 64- and 128-bit multiplication and division get markedly more expensive without native 64-bit registers.

---

# x86_32 Linux · GCC 14 (-m32) — 64 bits

![inline](img/perf_x32linux_gcc_64.png)

---

# x86_32 Linux · GCC 14 (-m32) — 128 bits

![inline](img/perf_x32linux_gcc_128.png)

---

# x86_64 Windows · MSVC — 32 bits

![inline](img/perf_x64win_msvc_32.png)

---

# x86_64 Windows · MSVC — 64 bits

![inline](img/perf_x64win_msvc_64.png)

---

# x86_64 Windows · MSVC — 128 bits

![inline](img/perf_x64win_msvc_128.png)

^ BID_UINT128 multiplication lands at 2.88× of decimal128_t here — 607× of hardware double.

---

# ARM64 Windows · MSVC

![inline](img/perf_arm64win_msvc.png)

^ No vendor decimal types exist for this target — MSVC ships no built-in decimal type, and Intel's library supports only IA-32, IA-64, and Intel x64 — so bars are relative to hardware double, and panels scale independently.

---

# ARM64 macOS · M4 Max

![inline](img/perf_m4mac_clang.png)

^ Homebrew Clang 22.1.7 on macOS Tahoe 26.5.1; Clang provides no built-in decimal types. Same normalization as the previous slide.

---

# `<charconv>` — x86_64 Windows · MSVC

![inline](img/perf_charconv_x64win.png)

^ The headline number: from_chars for decimal32_t at ~0.21× of MSVC's double from_chars. Both from_chars variants beat the hardware types for every decimal type except the 128-bit ones.

---

# `<charconv>` — ARM64 macOS

![inline](img/perf_charconv_m4mac.png)

^ from_chars trails double across the board here; to_chars wins at fixed precision and for 32/64-bit scientific shortest.
