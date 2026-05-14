#include "IIRFilter.h"
#include <algorithm>
#include <cstring>
#include <vector>
#include <complex>

namespace mygo_dsp {

IIRFilter::IIRFilter(const IIRFilterParams& params)
    : params_(params)
    , bypass_(false)
    , id_(-1)
{
    design(params);
}

void IIRFilter::reset() {
    for (auto& s : states_) {
        s.x1 = s.x2 = s.y1 = s.y2 = 0.0;
    }
}

bool IIRFilter::isBypassed() const {
    return bypass_;
}

void IIRFilter::setBypass(bool bypass) {
    bypass_ = bypass;
}

int IIRFilter::getId() const {
    return id_;
}

void IIRFilter::setId(int id) {
    id_ = id;
}

IIRFilterParams IIRFilter::getParams() const {
    return params_;
}

void IIRFilter::design(const IIRFilterParams& params) {
    params_ = params;
    computeBiquads();
    states_.resize(coeffs_.size());
    reset();
}

double IIRFilter::prewarp(double freq) const {
    return 2.0 * params_.sampleRate * std::tan(Pi * freq / params_.sampleRate);
}

void IIRFilter::butterworthPoles(int n, std::vector<std::complex<double>>& poles) {
    poles.resize(n);
    for (int k = 0; k < n; ++k) {
        double angle = Pi * (2.0 * k + 1.0) / (2.0 * n) + Pi / 2.0;
        poles[k] = std::exp(std::complex<double>(0.0, angle));
    }
}

void IIRFilter::chebyshevIPoles(int n, double ripple, std::vector<std::complex<double>>& poles) {
    double eps = std::sqrt(std::pow(10.0, ripple / 10.0) - 1.0);
    double a = std::asinh(1.0 / eps) / n;
    poles.resize(n);
    for (int k = 0; k < n; ++k) {
        double angle = Pi * (2.0 * k + 1.0) / (2.0 * n) + Pi / 2.0;
        double re = -std::sinh(a) * std::sin(angle - Pi / 2.0);
        double im = std::cosh(a) * std::cos(angle - Pi / 2.0);
        poles[k] = std::complex<double>(re, im);
    }
}

void IIRFilter::chebyshevIIPolesZeros(int n, double atten,
    std::vector<std::complex<double>>& poles, std::vector<std::complex<double>>& zeros) {
    double eps = 1.0 / std::sqrt(std::pow(10.0, atten / 10.0) - 1.0);
    double a = std::asinh(1.0 / eps) / n;
    poles.resize(n);
    zeros.resize(n);
    for (int k = 0; k < n; ++k) {
        double angle = Pi * (2.0 * k + 1.0) / (2.0 * n) + Pi / 2.0;
        double re = -std::sinh(a) * std::sin(angle - Pi / 2.0);
        double im = std::cosh(a) * std::cos(angle - Pi / 2.0);
        poles[k] = std::complex<double>(1.0, 0.0) / std::complex<double>(re, im);
        zeros[k] = std::complex<double>(0.0, 1.0) / std::cos(angle - Pi / 2.0);
    }
}

void IIRFilter::ellipticPolesZeros(int n, double ripple, double atten,
    std::vector<std::complex<double>>& poles, std::vector<std::complex<double>>& zeros) {
    double eps = std::sqrt(std::pow(10.0, ripple / 10.0) - 1.0);
    double k1 = eps / std::sqrt(std::pow(10.0, atten / 10.0) - 1.0);
    if (k1 > 1.0) k1 = 1.0 / k1;
    double k = 1.0 - 2.0 * std::pow((1.0 - k1) / (2.0 * (1.0 + k1)), 2.0) *
               std::pow(1.0 + std::sqrt(1.0 - std::pow((1.0 - k1) / (2.0 * (1.0 + k1)), 2.0)), -2.0);
    double K = 0.0, Kp = 0.0;
    {
        double q = (1.0 - std::sqrt(1.0 - k * k)) / (2.0 * (1.0 + std::sqrt(1.0 - k * k)));
        double sum = 1.0;
        double term = q;
        for (int i = 1; i <= 20; ++i) {
            sum += 2.0 * term;
            term *= q;
        }
        K = Pi / (2.0 * sum);
    }
    {
        double kp = std::sqrt(1.0 - k * k);
        double qp = (1.0 - std::sqrt(1.0 - kp * kp)) / (2.0 * (1.0 + std::sqrt(1.0 - kp * kp)));
        double sum = 1.0;
        double term = qp;
        for (int i = 1; i <= 20; ++i) {
            sum += 2.0 * term;
            term *= qp;
        }
        Kp = Pi / (2.0 * sum);
    }
    int N = (n % 2 == 0) ? n / 2 : (n - 1) / 2;
    poles.resize(2 * N);
    zeros.resize(n);
    double v0 = -std::asinh(1.0 / eps) / n;
    for (int i = 0; i < N; ++i) {
        double u = (2.0 * i + 1.0) * K / n;
        double sn = 0.0, cn = 0.0, dn = 0.0;
        {
            double m = k * k;
            double a0 = 1.0, b0 = std::sqrt(1.0 - m);
            double c0 = std::sqrt(m);
            double a = a0, b = b0, c = c0;
            int iter = 0;
            while (std::abs(c) > 1e-15 && iter < 20) {
                double an = (a + b) / 2.0;
                double bn = std::sqrt(a * b);
                double cn = (a - b) / 2.0;
                a = an; b = bn; c = cn;
                ++iter;
            }
            double phi = a0 * u;
            double t = std::tan(phi);
            sn = 2.0 * t / (1.0 + t * t);
        }
        double zeta_re = 0.0;
        double zeta_im = (2.0 * i + 1.0) * Kp / n;
        double pole_re = 0.0, pole_im = 0.0;
        {
            double num = 1.0 + sn * sn * v0 * v0;
            pole_re = -sn * std::sqrt(1.0 - sn * sn) * std::sinh(v0) / num;
            pole_im = std::sqrt(1.0 - sn * sn) * std::sqrt(1.0 + sn * sn * v0 * v0) * std::cosh(v0) / num;
        }
        poles[2*i] = std::complex<double>(pole_re, pole_im);
        poles[2*i+1] = std::complex<double>(pole_re, -pole_im);
        double t0 = std::sqrt((1.0 - k * sn * sn) / (1.0 - sn * sn / k / k));
        zeros[2*i] = std::complex<double>(0.0, 1.0 / (sn * k));
        zeros[2*i+1] = std::complex<double>(0.0, -1.0 / (sn * k));
    }
}

void IIRFilter::besselPoles(int n, std::vector<std::complex<double>>& poles) {
    poles.resize(n);
    static const double besselCoeffs[][8] = {
        {1.0},
        {1.0, 1.0},
        {3.0, 3.0, 1.0},
        {15.0, 15.0, 6.0, 1.0},
        {105.0, 105.0, 45.0, 10.0, 1.0},
        {945.0, 945.0, 420.0, 105.0, 15.0, 1.0},
        {10395.0, 10395.0, 4725.0, 1260.0, 210.0, 21.0, 1.0},
        {135135.0, 135135.0, 62370.0, 17325.0, 3150.0, 378.0, 28.0, 1.0}
    };
    if (n > 8) n = 8;
    std::vector<std::complex<double>> poly(n + 1);
    for (int i = 0; i <= n; ++i) {
        poly[i] = std::complex<double>(besselCoeffs[n-1][i], 0.0);
    }
    std::vector<std::complex<double>> roots;
    for (int iter = 0; iter < 1000; ++iter) {
        bool converged = true;
        for (int i = 0; i < n; ++i) {
            if (std::abs(poly[i]) < 1e-15) continue;
            std::complex<double> x = 0.5;
            for (int j = 0; j < 100; ++j) {
                std::complex<double> f = poly[n];
                std::complex<double> df = std::complex<double>(0.0, 0.0);
                for (int k = n - 1; k >= 0; --k) {
                    df = f + x * df;
                    f = poly[k] + x * f;
                }
                std::complex<double> dx = f / df;
                x = x - dx;
                if (std::abs(dx) < 1e-12) break;
            }
            roots.push_back(x);
            std::vector<std::complex<double>> newPoly(n);
            for (int k = 0; k < n; ++k) {
                newPoly[k] = poly[k+1] + x * poly[k];
            }
            for (int k = 0; k < n; ++k) poly[k] = newPoly[k];
            --n;
            converged = false;
            break;
        }
        if (converged) break;
    }
    for (size_t i = 0; i < poles.size(); ++i) {
        poles[i] = roots[i];
    }
}

void IIRFilter::legendrePoles(int n, std::vector<std::complex<double>>& poles) {
    butterworthPoles(n, poles);
    double scale = 1.0;
    for (int i = 0; i < n; ++i) {
        double r = std::abs(poles[i]);
        if (r > scale) scale = r;
    }
    for (int i = 0; i < n; ++i) {
        poles[i] /= scale;
    }
}

void IIRFilter::computeAnalogPrototype(int order, std::vector<std::complex<double>>& poles,
                                        std::vector<std::complex<double>>& zeros) {
    poles.clear();
    zeros.clear();
    switch (params_.prototype) {
        case IIRPrototype::Butterworth:
            butterworthPoles(order, poles);
            break;
        case IIRPrototype::ChebyshevI:
            chebyshevIPoles(order, params_.ripple, poles);
            break;
        case IIRPrototype::ChebyshevII:
            chebyshevIIPolesZeros(order, params_.stopbandAtten, poles, zeros);
            break;
        case IIRPrototype::Elliptic:
            ellipticPolesZeros(order, params_.ripple, params_.stopbandAtten, poles, zeros);
            break;
        case IIRPrototype::Bessel:
            besselPoles(order, poles);
            break;
        case IIRPrototype::Legendre:
            legendrePoles(order, poles);
            break;
        default:
            butterworthPoles(order, poles);
            break;
    }
}

void IIRFilter::sToZ(std::complex<double> pole, std::complex<double> zero,
                      double preWarpFreq, BiquadCoeffs& c) {
    std::complex<double> sp = pole * preWarpFreq;
    std::complex<double> sz = zero * preWarpFreq;
    double T = 1.0 / params_.sampleRate;
    std::complex<double> zp = (2.0 / T + sp) / (2.0 / T - sp);
    std::complex<double> zz = (2.0 / T + sz) / (2.0 / T - sz);
    double b0 = 1.0, b1 = -2.0 * zz.real(), b2 = std::norm(zz);
    double a0 = 1.0, a1 = -2.0 * zp.real(), a2 = std::norm(zp);
    double gain = std::norm(2.0 / T - sp) / std::norm(2.0 / T - sz);
    c.b0 = gain;
    c.b1 = gain * b1;
    c.b2 = gain * b2;
    c.a1 = a1;
    c.a2 = a2;
}

void IIRFilter::computeBiquads() {
    if (params_.prototype >= IIRPrototype::RBJ_LowPass &&
        params_.prototype <= IIRPrototype::RBJ_AllPass) {
        rbjDesign();
        return;
    }
    int order = params_.order;
    if (order < 1) order = 1;
    if (order > 20) order = 20;
    computeAnalogPrototype(order, analogPoles_, analogZeros_);
    bool hasZeros = !analogZeros_.empty();
    auto& poles = analogPoles_;
    auto& zeros = analogZeros_;
    int numSections = (order + 1) / 2;
    coeffs_.resize(numSections);
    double wc = prewarp(params_.cutoff);
    double wc2 = prewarp(params_.cutoff2);
    double w0 = (params_.passType == IIRPassType::LowPass) ? wc : wc;
    if (params_.passType == IIRPassType::BandPass || params_.passType == IIRPassType::BandStop) {
        w0 = std::sqrt(wc * wc2);
    }
    for (int i = 0; i < numSections; ++i) {
        int pi = 2 * i;
        if (pi >= order) pi = order - 1;
        std::complex<double> pole = poles[pi];
        std::complex<double> zero = (hasZeros && pi < static_cast<int>(zeros.size())) ? zeros[pi] : std::complex<double>(0.0, 0.0);
        if (params_.passType == IIRPassType::LowPass) {
            double fc = params_.cutoff / params_.sampleRate;
            double Q = 0.5 / std::abs(pole.real());
            double K = std::tan(Pi * fc);
            double K2 = K * K;
            double norm = 1.0 / (1.0 + K / Q + K2);
            coeffs_[i].b0 = K2 * norm;
            coeffs_[i].b1 = 2.0 * K2 * norm;
            coeffs_[i].b2 = K2 * norm;
            coeffs_[i].a1 = 2.0 * (K2 - 1.0) * norm;
            coeffs_[i].a2 = (1.0 - K / Q + K2) * norm;
        } else if (params_.passType == IIRPassType::HighPass) {
            double fc = params_.cutoff / params_.sampleRate;
            double Q = 0.5 / std::abs(pole.real());
            double K = std::tan(Pi * fc);
            double K2 = K * K;
            double norm = 1.0 / (1.0 + K / Q + K2);
            coeffs_[i].b0 = 1.0 * norm;
            coeffs_[i].b1 = -2.0 * norm;
            coeffs_[i].b2 = 1.0 * norm;
            coeffs_[i].a1 = 2.0 * (K2 - 1.0) * norm;
            coeffs_[i].a2 = (1.0 - K / Q + K2) * norm;
        } else if (params_.passType == IIRPassType::BandPass) {
            double fc = params_.cutoff;
            double fc2 = params_.cutoff2;
            double f0 = std::sqrt(fc * fc2);
            double bw = fc2 - fc;
            double Q = f0 / bw;
            double K = std::tan(Pi * f0 / params_.sampleRate);
            double K2 = K * K;
            double norm = 1.0 / (1.0 + K / Q + K2);
            coeffs_[i].b0 = (K / Q) * norm;
            coeffs_[i].b1 = 0.0;
            coeffs_[i].b2 = -(K / Q) * norm;
            coeffs_[i].a1 = 2.0 * (K2 - 1.0) * norm;
            coeffs_[i].a2 = (1.0 - K / Q + K2) * norm;
        } else if (params_.passType == IIRPassType::BandStop) {
            double fc = params_.cutoff;
            double fc2 = params_.cutoff2;
            double f0 = std::sqrt(fc * fc2);
            double bw = fc2 - fc;
            double Q = f0 / bw;
            double K = std::tan(Pi * f0 / params_.sampleRate);
            double K2 = K * K;
            double norm = 1.0 / (1.0 + K / Q + K2);
            coeffs_[i].b0 = (1.0 + K2) * norm;
            coeffs_[i].b1 = 2.0 * (K2 - 1.0) * norm;
            coeffs_[i].b2 = (1.0 + K2) * norm;
            coeffs_[i].a1 = 2.0 * (K2 - 1.0) * norm;
            coeffs_[i].a2 = (1.0 - K / Q + K2) * norm;
        }
    }
}

void IIRFilter::rbjDesign() {
    coeffs_.resize(1);
    double fs = params_.sampleRate;
    double f0 = params_.cutoff;
    double Q = params_.Q;
    if (Q < 1e-15) Q = 0.707;
    double A = std::sqrt(params_.shelfGain);
    double w0 = TwoPi * f0 / fs;
    double cosW0 = std::cos(w0);
    double sinW0 = std::sin(w0);
    double alpha = sinW0 / (2.0 * Q);
    switch (params_.prototype) {
        case IIRPrototype::RBJ_LowPass: {
            double b0 = (1.0 - cosW0) / 2.0;
            double b1 = 1.0 - cosW0;
            double b2 = (1.0 - cosW0) / 2.0;
            double a0 = 1.0 + alpha;
            double a1 = -2.0 * cosW0;
            double a2 = 1.0 - alpha;
            coeffs_[0].b0 = b0 / a0;
            coeffs_[0].b1 = b1 / a0;
            coeffs_[0].b2 = b2 / a0;
            coeffs_[0].a1 = a1 / a0;
            coeffs_[0].a2 = a2 / a0;
            break;
        }
        case IIRPrototype::RBJ_HighPass: {
            double b0 = (1.0 + cosW0) / 2.0;
            double b1 = -(1.0 + cosW0);
            double b2 = (1.0 + cosW0) / 2.0;
            double a0 = 1.0 + alpha;
            double a1 = -2.0 * cosW0;
            double a2 = 1.0 - alpha;
            coeffs_[0].b0 = b0 / a0;
            coeffs_[0].b1 = b1 / a0;
            coeffs_[0].b2 = b2 / a0;
            coeffs_[0].a1 = a1 / a0;
            coeffs_[0].a2 = a2 / a0;
            break;
        }
        case IIRPrototype::RBJ_BandPass1: {
            double b0 = sinW0 / 2.0;
            double b1 = 0.0;
            double b2 = -sinW0 / 2.0;
            double a0 = 1.0 + alpha;
            double a1 = -2.0 * cosW0;
            double a2 = 1.0 - alpha;
            coeffs_[0].b0 = b0 / a0;
            coeffs_[0].b1 = b1 / a0;
            coeffs_[0].b2 = b2 / a0;
            coeffs_[0].a1 = a1 / a0;
            coeffs_[0].a2 = a2 / a0;
            break;
        }
        case IIRPrototype::RBJ_BandPass2: {
            double b0 = alpha;
            double b1 = 0.0;
            double b2 = -alpha;
            double a0 = 1.0 + alpha;
            double a1 = -2.0 * cosW0;
            double a2 = 1.0 - alpha;
            coeffs_[0].b0 = b0 / a0;
            coeffs_[0].b1 = b1 / a0;
            coeffs_[0].b2 = b2 / a0;
            coeffs_[0].a1 = a1 / a0;
            coeffs_[0].a2 = a2 / a0;
            break;
        }
        case IIRPrototype::RBJ_BandStop: {
            double b0 = 1.0;
            double b1 = -2.0 * cosW0;
            double b2 = 1.0;
            double a0 = 1.0 + alpha;
            double a1 = -2.0 * cosW0;
            double a2 = 1.0 - alpha;
            coeffs_[0].b0 = b0 / a0;
            coeffs_[0].b1 = b1 / a0;
            coeffs_[0].b2 = b2 / a0;
            coeffs_[0].a1 = a1 / a0;
            coeffs_[0].a2 = a2 / a0;
            break;
        }
        case IIRPrototype::RBJ_LowShelf: {
            double ap = std::pow(A, 0.5) * alpha;
            double b0 = A * ((A + 1.0) - (A - 1.0) * cosW0 + 2.0 * ap);
            double b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cosW0);
            double b2 = A * ((A + 1.0) - (A - 1.0) * cosW0 - 2.0 * ap);
            double a0 = (A + 1.0) + (A - 1.0) * cosW0 + 2.0 * ap;
            double a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cosW0);
            double a2 = (A + 1.0) + (A - 1.0) * cosW0 - 2.0 * ap;
            coeffs_[0].b0 = b0 / a0;
            coeffs_[0].b1 = b1 / a0;
            coeffs_[0].b2 = b2 / a0;
            coeffs_[0].a1 = a1 / a0;
            coeffs_[0].a2 = a2 / a0;
            break;
        }
        case IIRPrototype::RBJ_HighShelf: {
            double ap = std::pow(A, 0.5) * alpha;
            double b0 = A * ((A + 1.0) + (A - 1.0) * cosW0 + 2.0 * ap);
            double b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cosW0);
            double b2 = A * ((A + 1.0) + (A - 1.0) * cosW0 - 2.0 * ap);
            double a0 = (A + 1.0) - (A - 1.0) * cosW0 + 2.0 * ap;
            double a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cosW0);
            double a2 = (A + 1.0) - (A - 1.0) * cosW0 - 2.0 * ap;
            coeffs_[0].b0 = b0 / a0;
            coeffs_[0].b1 = b1 / a0;
            coeffs_[0].b2 = b2 / a0;
            coeffs_[0].a1 = a1 / a0;
            coeffs_[0].a2 = a2 / a0;
            break;
        }
        case IIRPrototype::RBJ_BandShelf: {
            double S = params_.Q;
            if (S < 1e-15) S = 1.0;
            double bw = S;
            double ap = sinW0 * std::sinh(std::log(2.0) / 2.0 * bw * w0 / sinW0);
            double b0 = A * ((A + 1.0) - (A - 1.0) * cosW0 + 2.0 * ap);
            double b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cosW0);
            double b2 = A * ((A + 1.0) - (A - 1.0) * cosW0 - 2.0 * ap);
            double a0 = (A + 1.0) + (A - 1.0) * cosW0 + 2.0 * ap;
            double a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cosW0);
            double a2 = (A + 1.0) + (A - 1.0) * cosW0 - 2.0 * ap;
            coeffs_[0].b0 = b0 / a0;
            coeffs_[0].b1 = b1 / a0;
            coeffs_[0].b2 = b2 / a0;
            coeffs_[0].a1 = a1 / a0;
            coeffs_[0].a2 = a2 / a0;
            break;
        }
        case IIRPrototype::RBJ_AllPass: {
            double b0 = 1.0 - alpha;
            double b1 = -2.0 * cosW0;
            double b2 = 1.0 + alpha;
            double a0 = 1.0 + alpha;
            double a1 = -2.0 * cosW0;
            double a2 = 1.0 - alpha;
            coeffs_[0].b0 = b0 / a0;
            coeffs_[0].b1 = b1 / a0;
            coeffs_[0].b2 = b2 / a0;
            coeffs_[0].a1 = a1 / a0;
            coeffs_[0].a2 = a2 / a0;
            break;
        }
        default:
            break;
    }
}

void IIRFilter::process(const double* input, double* output, size_t numSamples) {
    if (bypass_) {
        if (input != output) {
            std::copy(input, input + numSamples, output);
        }
        return;
    }
    size_t numBiquads = coeffs_.size();
    for (size_t i = 0; i < numSamples; ++i) {
        double x = input[i];
        for (size_t j = 0; j < numBiquads; ++j) {
            const auto& c = coeffs_[j];
            auto& s = states_[j];
            double y = c.b0 * x + c.b1 * s.x1 + c.b2 * s.x2
                       - c.a1 * s.y1 - c.a2 * s.y2;
            s.x2 = s.x1;
            s.x1 = x;
            s.y2 = s.y1;
            s.y1 = y;
            x = y;
        }
        output[i] = x;
    }
}

void IIRFilter::getPoleZero(std::vector<std::complex<double>>& poles,
                             std::vector<std::complex<double>>& zeros) const {
    poles = analogPoles_;
    zeros = analogZeros_;
}

} // namespace mygo_dsp
