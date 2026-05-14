#pragma once
#ifndef WAVEFORMGENERATOR_H
#define WAVEFORMGENERATOR_H

#include "IOperator.h"
#include "DSPMath.h"
#include <vector>
#include <cmath>
#include <random>
#include <cstdint>

namespace mygo_dsp {

enum class WaveformType {
    Sine,
    Square,
    Triangle,
    Sawtooth,
    WhiteNoise,
    PinkNoise,
    Pulse
};

struct WaveformParams {
    WaveformType type = WaveformType::Sine;
    double frequency = 440.0;
    double amplitude = 1.0;
    double phase = 0.0;
    double dutyCycle = 0.5;
    double sampleRate = 48000.0;
};

class WaveformGenerator : public IOperator {
public:
    explicit WaveformGenerator(const WaveformParams& params = WaveformParams{});

    void process(const double* input, double* output, size_t numSamples) override;

    void reset() override;

    bool isBypassed() const override;
    void setBypass(bool bypass) override;

    void setParams(const WaveformParams& params);
    WaveformParams getParams() const;

    int getId() const;
    void setId(int id);

private:
    double generateSample(double t) const;

    WaveformParams params_;
    bool bypass_;
    int id_;
    double phaseAccum_;
    mutable std::mt19937_64 rng_;
    mutable double pinkB0_, pinkB1_, pinkB2_, pinkB3_, pinkB4_, pinkB5_, pinkB6_;
    mutable int pinkState_;
};

} // namespace mygo_dsp

#endif // WAVEFORMGENERATOR_H
