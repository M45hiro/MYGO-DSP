#pragma once
#ifndef IIRFILTER_H
#define IIRFILTER_H

#include "IOperator.h"
#include "DSPMath.h"
#include <vector>
#include <cmath>
#include <complex>

namespace mygo_dsp {

enum class IIRPrototype {
    Butterworth,
    ChebyshevI,
    ChebyshevII,
    Elliptic,
    Bessel,
    Legendre,
    RBJ_LowPass,
    RBJ_HighPass,
    RBJ_BandPass1,
    RBJ_BandPass2,
    RBJ_BandStop,
    RBJ_LowShelf,
    RBJ_HighShelf,
    RBJ_BandShelf,
    RBJ_AllPass
};

enum class IIRPassType {
    LowPass,
    HighPass,
    BandPass,
    BandStop
};

struct BiquadCoeffs {
    double b0 = 1.0, b1 = 0.0, b2 = 0.0;
    double a0 = 1.0, a1 = 0.0, a2 = 0.0;
};

struct BiquadState {
    double x1 = 0.0, x2 = 0.0;
    double y1 = 0.0, y2 = 0.0;
};

struct IIRFilterParams {
    IIRPrototype prototype = IIRPrototype::Butterworth;
    IIRPassType passType = IIRPassType::LowPass;
    int order = 4;
    double cutoff = 1000.0;
    double cutoff2 = 4000.0;
    double sampleRate = 48000.0;
    double ripple = 0.5;
    double stopbandAtten = 40.0;
    double shelfGain = 1.0;
    double Q = 0.707;
};

class IIRFilter : public IOperator {
public:
    explicit IIRFilter(const IIRFilterParams& params = IIRFilterParams{});

    void process(const double* input, double* output, size_t numSamples) override;

    void reset() override;

    bool isBypassed() const override;
    void setBypass(bool bypass) override;

    void design(const IIRFilterParams& params);
    IIRFilterParams getParams() const;
    void getPoleZero(std::vector<std::complex<double>>& poles,
                     std::vector<std::complex<double>>& zeros) const;

    int getId() const;
    void setId(int id);

private:
    void computeBiquads();
    void computeAnalogPrototype(int order, std::vector<std::complex<double>>& poles,
                                 std::vector<std::complex<double>>& zeros);
    std::vector<std::complex<double>> analogPoles_;
    std::vector<std::complex<double>> analogZeros_;
    void butterworthPoles(int n, std::vector<std::complex<double>>& poles);
    void chebyshevIPoles(int n, double ripple, std::vector<std::complex<double>>& poles);
    void chebyshevIIPolesZeros(int n, double atten, std::vector<std::complex<double>>& poles,
                                std::vector<std::complex<double>>& zeros);
    void ellipticPolesZeros(int n, double ripple, double atten, std::vector<std::complex<double>>& poles,
                             std::vector<std::complex<double>>& zeros);
    void besselPoles(int n, std::vector<std::complex<double>>& poles);
    void legendrePoles(int n, std::vector<std::complex<double>>& poles);

    void sToZ(std::complex<double> pole, std::complex<double> zero,
              double preWarpFreq, BiquadCoeffs& coeffs);

    void rbjDesign();

    double prewarp(double freq) const;

    IIRFilterParams params_;
    std::vector<BiquadCoeffs> coeffs_;
    std::vector<BiquadState> states_;
    bool bypass_;
    int id_;
};

} // namespace mygo_dsp

#endif // IIRFILTER_H
