#pragma once
#include "IIRFilter.h"
#include "FIRFilter.h"
#include <string>
#include <vector>
#include <cmath>

namespace mygo_dsp {

struct FilterSpec {
    enum Type { LowPass, HighPass, BandPass, BandStop };
    Type type = LowPass;
    double fs = 44100.0;
    double fpass = 1000.0;
    double fstop = 2000.0;
    double fpass2 = 0.0;
    double fstop2 = 0.0;
    double Ap = 1.0;
    double As = 40.0;
};

struct AutoDesignResult {
    bool success = false;
    std::string warning;
    IIRPrototype selectedProto = IIRPrototype::Butterworth;
    int order = 0;
    bool useFIR = false;
    FilterPassType firPassType = FilterPassType::LowPass;
    WindowType firWindow = WindowType::Kaiser;
    int firOrder = 0;
    double firBeta = 0.0;
};

class AutoFilterDesigner {
public:
    static AutoDesignResult autoDesign(const FilterSpec& spec, int maxOrder = 20);
private:
    static void normalizeFrequencies(const FilterSpec& spec, double& wp, double& ws);
    static int estimateOrderButterworth(double wp, double ws, double Ap, double As);
    static int estimateOrderChebyshev(double wp, double ws, double Ap, double As);
    static int estimateOrderElliptic(double wp, double ws, double Ap, double As);
    static double ellipk(double k);
    static AutoDesignResult designFIRKaiser(const FilterSpec& spec, int& order, double& beta);
    static void designAndVerify(IIRPrototype proto, int order, const FilterSpec& spec, AutoDesignResult& result);
};

} // namespace mygo_dsp
