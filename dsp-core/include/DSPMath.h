#pragma once
#ifndef DSPMATH_H
#define DSPMATH_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

namespace mygo_dsp {

constexpr double Pi = M_PI;
constexpr double TwoPi = 2.0 * M_PI;
constexpr double kPi = M_PI;        // for compatibility
constexpr double kTwoPi = 2.0 * M_PI;

} // namespace mygo_dsp

#endif // DSPMATH_H
