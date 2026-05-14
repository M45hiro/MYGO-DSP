#include "AutoFilterDesigner.h"
#include "DSPMath.h"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace mygo_dsp {

static double pow10(double x) { return std::pow(10.0, x); }

AutoDesignResult AutoFilterDesigner::autoDesign(const FilterSpec& spec, int maxOrder) {
    AutoDesignResult result;

    double wp = 0.0, ws = 0.0;
    normalizeFrequencies(spec, wp, ws);

    if (spec.type == FilterSpec::LowPass || spec.type == FilterSpec::HighPass) {
        if (wp >= ws) {
            result.success = false;
            result.warning = "Passband frequency must be less than stopband frequency";
            return result;
        }
    }
    if (spec.type == FilterSpec::BandPass || spec.type == FilterSpec::BandStop) {
        if (spec.fpass >= spec.fstop) {
            result.success = false;
            result.warning = "Lower cutoff frequency must be less than upper cutoff frequency";
            return result;
        }
    }

    for (auto proto : {IIRPrototype::Elliptic, IIRPrototype::ChebyshevII,
                       IIRPrototype::Butterworth}) {
        int N = 0;
        switch (proto) {
        case IIRPrototype::Butterworth: N = estimateOrderButterworth(wp, ws, spec.Ap, spec.As); break;
        case IIRPrototype::ChebyshevII: N = estimateOrderChebyshev(wp, ws, spec.Ap, spec.As); break;
        case IIRPrototype::Elliptic:    N = estimateOrderElliptic(wp, ws, spec.Ap, spec.As); break;
        default: break;
        }
        if (N > 0 && N <= maxOrder) {
            designAndVerify(proto, N, spec, result);
            if (result.success) {
                result.selectedProto = proto;
                result.order = N;
                return result;
            }
        }
    }

    // Fallback: Kaiser-window FIR
    result.warning = "IIR filters exceed max order. Falling back to Kaiser FIR.";
    result.useFIR = true;
    return designFIRKaiser(spec, result.firOrder, result.firBeta);
}

void AutoFilterDesigner::normalizeFrequencies(const FilterSpec& spec, double& wp, double& ws) {
    double fs2 = spec.fs / 2.0;
    switch (spec.type) {
    case FilterSpec::LowPass:  wp = spec.fpass/fs2; ws = spec.fstop/fs2; break;
    case FilterSpec::HighPass: wp = 1.0 - spec.fpass/fs2; ws = 1.0 - spec.fstop/fs2; break;
    case FilterSpec::BandPass: {
        double f0 = std::sqrt(spec.fpass * spec.fpass2);
        double bw = spec.fpass2 - spec.fpass;
        double sbw = spec.fstop2 - spec.fstop;
        wp = bw / f0;
        ws = sbw / f0;
        break;
    }
    case FilterSpec::BandStop: {
        double f0 = std::sqrt(spec.fpass * spec.fpass2);
        double bw = spec.fpass2 - spec.fpass;
        double sbw = spec.fstop2 - spec.fstop;
        ws = bw / f0;
        wp = sbw / f0;
        break;
    }
    }
    wp = std::max(1e-6, std::min(wp, 1.0 - 1e-6));
    ws = std::max(1e-6, std::min(ws, 1.0 - 1e-6));
}

int AutoFilterDesigner::estimateOrderButterworth(double wp, double ws, double Ap, double As) {
    double delta = std::sqrt((pow10(As/10.0) - 1.0) / (pow10(Ap/10.0) - 1.0));
    double ratio = ws / wp;
    if (ratio <= 1.0) return 999;
    return static_cast<int>(std::ceil(std::log10(delta) / (2.0 * std::log10(ratio))));
}

int AutoFilterDesigner::estimateOrderChebyshev(double wp, double ws, double Ap, double As) {
    double delta = std::sqrt((pow10(As/10.0) - 1.0) / (pow10(Ap/10.0) - 1.0));
    double ratio = ws / wp;
    if (ratio <= 1.0) return 999;
    auto acosh = [](double x) { return std::log(x + std::sqrt(x*x - 1.0)); };
    return static_cast<int>(std::ceil(acosh(delta) / acosh(ratio)));
}

double AutoFilterDesigner::ellipk(double k) {
    if (k < 0 || k >= 1) return (k >= 1) ? 1e10 : kPi / 2.0;
    double a = 1.0, b = std::sqrt(1.0 - k*k), c = k;
    while (std::abs(c) > 1e-15) {
        double an = (a + b) / 2.0, bn = std::sqrt(a*b);
        c = (a - b) / 2.0; a = an; b = bn;
    }
    return kPi / (2.0 * a);
}

int AutoFilterDesigner::estimateOrderElliptic(double wp, double ws, double Ap, double As) {
    double k = wp / ws;
    if (k >= 1.0) return 999;
    double eps_p = std::sqrt(pow10(Ap/10.0) - 1.0);
    double eps_s = std::sqrt(pow10(As/10.0) - 1.0);
    double k1 = eps_p / eps_s;
    if (k1 >= 1.0) k1 = eps_s / eps_p;
    double Kk = ellipk(k), Kk1 = ellipk(k1);
    double Kc = ellipk(std::sqrt(1.0 - k*k)), K1c = ellipk(std::sqrt(1.0 - k1*k1));
    if (Kc < 1e-10 || K1c < 1e-10) return 999;
    return static_cast<int>(std::ceil((Kk * K1c) / (Kc * Kk1)));
}

void AutoFilterDesigner::designAndVerify(IIRPrototype proto, int order, const FilterSpec& spec,
                                          AutoDesignResult& result) {
    IIRFilterParams params;
    params.prototype = proto;
    params.order = order;
    params.cutoff = spec.fpass;
    params.cutoff2 = (spec.fpass2 > 0) ? spec.fpass2 : spec.fpass * 2.0;
    params.sampleRate = spec.fs;
    params.ripple = spec.Ap;
    params.stopbandAtten = spec.As;
    params.shelfGain = 1.0;
    params.Q = 0.707;
    switch (spec.type) {
    case FilterSpec::LowPass:  params.passType = IIRPassType::LowPass; break;
    case FilterSpec::HighPass: params.passType = IIRPassType::HighPass; break;
    case FilterSpec::BandPass: params.passType = IIRPassType::BandPass; break;
    case FilterSpec::BandStop: params.passType = IIRPassType::BandStop; break;
    }
    IIRFilter filter(params);
    result.success = true;
    result.order = order;
    result.selectedProto = proto;
}

AutoDesignResult AutoFilterDesigner::designFIRKaiser(const FilterSpec& spec,
                                                      int& order, double& beta) {
    AutoDesignResult result;
    result.useFIR = true;
    double dw = 2.0 * kPi * std::abs(spec.fstop - spec.fpass) / spec.fs;
    double As = spec.As;
    order = static_cast<int>(std::ceil((As - 7.95) / (14.36 * dw / (2.0 * kPi))));
    order = std::max(order, 3);
    if (order % 2 == 0) ++order;
    if (As > 50.0) beta = 0.1102 * (As - 8.7);
    else if (As >= 21.0) beta = 0.5842 * std::pow(As - 21.0, 0.4) + 0.07886 * (As - 21.0);
    else beta = 0.0;
    result.firOrder = order;
    result.firBeta = beta;
    result.success = true;
    switch (spec.type) {
    case FilterSpec::LowPass:  result.firPassType = FilterPassType::LowPass; break;
    case FilterSpec::HighPass: result.firPassType = FilterPassType::HighPass; break;
    case FilterSpec::BandPass: result.firPassType = FilterPassType::BandPass; break;
    case FilterSpec::BandStop: result.firPassType = FilterPassType::BandStop; break;
    }
    return result;
}

} // namespace mygo_dsp
