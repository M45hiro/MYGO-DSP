#include "WaveformGenerator.h"

namespace mygo_dsp {

WaveformGenerator::WaveformGenerator(const WaveformParams& params)
    : params_(params)
    , bypass_(false)
    , id_(-1)
    , phaseAccum_(params.phase)
    , rng_(std::random_device{}())
    , pinkB0_(0), pinkB1_(0), pinkB2_(0), pinkB3_(0), pinkB4_(0), pinkB5_(0), pinkB6_(0)
    , pinkState_(0)
{}

void WaveformGenerator::setParams(const WaveformParams& params) {
    params_ = params;
}

WaveformParams WaveformGenerator::getParams() const {
    return params_;
}

int WaveformGenerator::getId() const {
    return id_;
}

void WaveformGenerator::setId(int id) {
    id_ = id;
}

bool WaveformGenerator::isBypassed() const {
    return bypass_;
}

void WaveformGenerator::setBypass(bool bypass) {
    bypass_ = bypass;
}

void WaveformGenerator::reset() {
    phaseAccum_ = params_.phase;
    pinkB0_ = pinkB1_ = pinkB2_ = pinkB3_ = pinkB4_ = pinkB5_ = pinkB6_ = 0;
    pinkState_ = 0;
}

double WaveformGenerator::generateSample(double t) const {
    double val = 0.0;
    switch (params_.type) {
        case WaveformType::Sine:
            val = std::sin(TwoPi * t + params_.phase);
            break;
        case WaveformType::Square:
            val = (std::fmod(t + params_.phase / TwoPi, 1.0) < params_.dutyCycle) ? 1.0 : -1.0;
            break;
        case WaveformType::Triangle: {
            double p = std::fmod(t + params_.phase / TwoPi, 1.0);
            val = 4.0 * std::abs(p - 0.5) - 1.0;
            break;
        }
        case WaveformType::Sawtooth: {
            double p = std::fmod(t + params_.phase / TwoPi, 1.0);
            val = 2.0 * p - 1.0;
            break;
        }
        case WaveformType::WhiteNoise: {
            std::mt19937_64& rng = const_cast<std::mt19937_64&>(rng_);
            std::uniform_real_distribution<double> dist(-1.0, 1.0);
            val = dist(rng);
            break;
        }
        case WaveformType::PinkNoise: {
            std::mt19937_64& rng = const_cast<std::mt19937_64&>(rng_);
            std::uniform_real_distribution<double> dist(-1.0, 1.0);
            double white = dist(rng);
            pinkB0_ = 0.99886 * pinkB0_ + white * 0.0555179;
            pinkB1_ = 0.99332 * pinkB1_ + white * 0.0750759;
            pinkB2_ = 0.96900 * pinkB2_ + white * 0.1538520;
            pinkB3_ = 0.86650 * pinkB3_ + white * 0.3104856;
            pinkB4_ = 0.55000 * pinkB4_ + white * 0.5329522;
            pinkB5_ = -0.7616 * pinkB5_ - white * 0.0168980;
            double pink = pinkB0_ + pinkB1_ + pinkB2_ + pinkB3_ + pinkB4_ + pinkB5_ + pinkB6_ + white * 0.5362;
            pinkB6_ = white * 0.115926;
            val = pink * 0.11;
            if (val > 1.0) val = 1.0;
            if (val < -1.0) val = -1.0;
            break;
        }
        case WaveformType::Pulse: {
            double p = std::fmod(t + params_.phase / TwoPi, 1.0);
            val = (p < params_.dutyCycle) ? params_.amplitude : 0.0;
            break;
        }
    }
    return val * params_.amplitude;
}

void WaveformGenerator::process(const double* input, double* output, size_t numSamples) {
    if (bypass_) {
        if (input) {
            if (input != output) {
                std::copy(input, input + numSamples, output);
            }
        } else {
            std::fill(output, output + numSamples, 0.0);
        }
        return;
    }
    double phaseIncrement = params_.frequency / params_.sampleRate;
    for (size_t i = 0; i < numSamples; ++i) {
        double t = phaseAccum_;
        output[i] = generateSample(t) + (input ? input[i] : 0.0);
        phaseAccum_ += phaseIncrement;
        if (phaseAccum_ >= 1.0) phaseAccum_ -= std::floor(phaseAccum_);
    }
}

} // namespace mygo_dsp
