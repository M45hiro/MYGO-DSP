#include "FIRFilter.h"
#include <algorithm>
#include <cstring>
#include <complex>

namespace mygo_dsp {

FIRFilter::FIRFilter(const FIRFilterParams& params)
    : params_(params)
    , bypass_(false)
    , id_(-1)
    , delayIndex_(0)
{
    design(params);
}

void FIRFilter::reset() {
    std::fill(delayLine_.begin(), delayLine_.end(), 0.0);
    delayIndex_ = 0;
}

bool FIRFilter::isBypassed() const {
    return bypass_;
}

void FIRFilter::setBypass(bool bypass) {
    bypass_ = bypass;
}

int FIRFilter::getId() const {
    return id_;
}

void FIRFilter::setId(int id) {
    id_ = id;
}

FIRFilterParams FIRFilter::getParams() const {
    return params_;
}

void FIRFilter::design(const FIRFilterParams& params) {
    params_ = params;
    int n = params_.order + 1;
    taps_.resize(n);
    delayLine_.resize(n, 0.0);
    delayIndex_ = 0;

    switch (params_.passType) {
        case FilterPassType::LowPass: designLowPass(); break;
        case FilterPassType::HighPass: designHighPass(); break;
        case FilterPassType::BandPass: designBandPass(); break;
        case FilterPassType::BandStop: designBandStop(); break;
    }

    normalizeTaps();

    switch (params_.window) {
        case WindowType::Rectangular: designRectangular(); break;
        case WindowType::Hamming: designHamming(); break;
        case WindowType::Hanning: designHanning(); break;
        case WindowType::Blackman: designBlackman(); break;
        case WindowType::Kaiser: designKaiser(); break;
    }

    normalizeTaps();
}

double FIRFilter::kaiserI0(double x) const {
    double sum = 1.0;
    double term = 1.0;
    for (int k = 1; k <= 50; ++k) {
        term *= (x * x) / (4.0 * k * k);
        sum += term;
        if (std::abs(term) < 1e-15) break;
    }
    return sum;
}

void FIRFilter::designLowPass() {
    int n = static_cast<int>(taps_.size());
    int M = n / 2;
    double fc = params_.cutoff / params_.sampleRate;
    for (int i = 0; i < n; ++i) {
        int k = i - M;
        if (k == 0) {
            taps_[i] = 2.0 * fc;
        } else {
            taps_[i] = std::sin(TwoPi * fc * k) / (Pi * k);
        }
    }
}

void FIRFilter::designHighPass() {
    int n = static_cast<int>(taps_.size());
    int M = n / 2;
    double fc = params_.cutoff / params_.sampleRate;
    for (int i = 0; i < n; ++i) {
        int k = i - M;
        if (k == 0) {
            taps_[i] = 1.0 - 2.0 * fc;
        } else {
            taps_[i] = -std::sin(TwoPi * fc * k) / (Pi * k);
        }
    }
}

void FIRFilter::designBandPass() {
    int n = static_cast<int>(taps_.size());
    int M = n / 2;
    double fc1 = params_.cutoff / params_.sampleRate;
    double fc2 = params_.cutoff2 / params_.sampleRate;
    for (int i = 0; i < n; ++i) {
        int k = i - M;
        if (k == 0) {
            taps_[i] = 2.0 * (fc2 - fc1);
        } else {
            taps_[i] = (std::sin(TwoPi * fc2 * k) - std::sin(TwoPi * fc1 * k)) / (Pi * k);
        }
    }
}

void FIRFilter::designBandStop() {
    int n = static_cast<int>(taps_.size());
    int M = n / 2;
    double fc1 = params_.cutoff / params_.sampleRate;
    double fc2 = params_.cutoff2 / params_.sampleRate;
    for (int i = 0; i < n; ++i) {
        int k = i - M;
        if (k == 0) {
            taps_[i] = 1.0 - 2.0 * (fc2 - fc1);
        } else {
            taps_[i] = (std::sin(TwoPi * fc1 * k) - std::sin(TwoPi * fc2 * k)) / (Pi * k);
        }
    }
}

void FIRFilter::normalizeTaps() {
    double sum = 0.0;
    for (double t : taps_) sum += t;
    if (std::abs(sum) > 1e-15) {
        for (double& t : taps_) t /= sum;
    }
}

void FIRFilter::designRectangular() {
    // Already designed, rectangular is identity window
}

void FIRFilter::designHamming() {
    int n = static_cast<int>(taps_.size());
    for (int i = 0; i < n; ++i) {
        double w = 0.54 - 0.46 * std::cos(TwoPi * i / (n - 1));
        taps_[i] *= w;
    }
}

void FIRFilter::designHanning() {
    int n = static_cast<int>(taps_.size());
    for (int i = 0; i < n; ++i) {
        double w = 0.5 - 0.5 * std::cos(TwoPi * i / (n - 1));
        taps_[i] *= w;
    }
}

void FIRFilter::designBlackman() {
    int n = static_cast<int>(taps_.size());
    for (int i = 0; i < n; ++i) {
        double w = 0.42 - 0.5 * std::cos(TwoPi * i / (n - 1))
                   + 0.08 * std::cos(4.0 * Pi * i / (n - 1));
        taps_[i] *= w;
    }
}

void FIRFilter::designKaiser() {
    int n = static_cast<int>(taps_.size());
    double beta = params_.kaiserBeta;
    double iz0 = kaiserI0(beta);
    for (int i = 0; i < n; ++i) {
        double m = 2.0 * i / (n - 1) - 1.0;
        double arg = beta * std::sqrt(1.0 - m * m);
        double w = kaiserI0(arg) / iz0;
        taps_[i] *= w;
    }
}

void FIRFilter::process(const double* input, double* output, size_t numSamples) {
    if (bypass_) {
        if (input != output) {
            std::copy(input, input + numSamples, output);
        }
        return;
    }

    int n = static_cast<int>(taps_.size());
    for (size_t i = 0; i < numSamples; ++i) {
        delayLine_[delayIndex_] = input[i];
        double sum = 0.0;
        size_t idx = delayIndex_;
        for (int j = 0; j < n; ++j) {
            sum += taps_[j] * delayLine_[idx];
            if (idx == 0) idx = static_cast<size_t>(n) - 1;
            else --idx;
        }
        output[i] = sum;
        delayIndex_ = (delayIndex_ + 1) % n;
    }
}

void FIRFilter::getZeros(std::vector<std::complex<double>>& zeros) const {
    zeros.clear();
    if (taps_.empty()) return;
    int order = static_cast<int>(taps_.size()) - 1;
    if (order < 1) return;
    std::vector<std::complex<double>> coeffs(order + 1);
    for (int i = 0; i <= order; ++i) coeffs[i] = taps_[i];
    for (int i = 0; i < order; ++i) {
        if (std::abs(coeffs[i]) < 1e-15 && i + 1 < order) continue;
        std::complex<double> x(0.0, 0.0);
        for (int iter = 0; iter < 200; ++iter) {
            std::complex<double> f = coeffs[order];
            std::complex<double> df(0.0, 0.0);
            for (int k = order - 1; k >= 0; --k) {
                df = f + x * df;
                f = coeffs[k] + x * f;
            }
            if (std::abs(df) < 1e-15) break;
            x -= f / df;
        }
        zeros.push_back(x);
        for (int k = order - 1; k >= 0; --k)
            coeffs[k + 1] += x * coeffs[k];
        --order;
    }
}

} // namespace mygo_dsp
