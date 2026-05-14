#pragma once
#ifndef FIRFILTER_H
#define FIRFILTER_H

#include "IOperator.h"
#include "DSPMath.h"
#include <vector>
#include <cmath>
#include <functional>
#include <complex>

namespace mygo_dsp {

enum class WindowType {
    Rectangular,
    Hamming,
    Hanning,
    Blackman,
    Kaiser
};

enum class FilterPassType {
    LowPass,
    HighPass,
    BandPass,
    BandStop
};

struct FIRFilterParams {
    WindowType window = WindowType::Hamming;
    FilterPassType passType = FilterPassType::LowPass;
    int order = 64;
    double cutoff = 1000.0;
    double cutoff2 = 4000.0;
    double sampleRate = 48000.0;
    double kaiserBeta = 0.5;
};

class FIRFilter : public IOperator {
public:
    explicit FIRFilter(const FIRFilterParams& params = FIRFilterParams{});

    void process(const double* input, double* output, size_t numSamples) override;

    void reset() override;

    bool isBypassed() const override;
    void setBypass(bool bypass) override;

    void design(const FIRFilterParams& params);
    FIRFilterParams getParams() const;
    void getZeros(std::vector<std::complex<double>>& zeros) const;

    int getId() const;
    void setId(int id);

private:
    void designRectangular();
    void designHamming();
    void designHanning();
    void designBlackman();
    void designKaiser();

    double kaiserI0(double x) const;
    void designLowPass();
    void designHighPass();
    void designBandPass();
    void designBandStop();
    void normalizeTaps();

    FIRFilterParams params_;
    std::vector<double> taps_;
    std::vector<double> delayLine_;
    bool bypass_;
    int id_;
    size_t delayIndex_;
};

} // namespace mygo_dsp

#endif // FIRFILTER_H
