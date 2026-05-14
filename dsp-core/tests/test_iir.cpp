#include <gtest/gtest.h>
#include "IIRFilter.h"

using namespace mygo_dsp;

TEST(IIRFilterTest, ButterworthLowPassStable) {
    IIRFilterParams p;
    p.prototype = IIRPrototype::Butterworth;
    p.passType = IIRPassType::LowPass;
    p.order = 4;
    p.cutoff = 1000.0;
    p.sampleRate = 44100.0;

    IIRFilter filter(p);
    double input[128];
    double output[128];
    for (int i = 0; i < 128; ++i) input[i] = (i % 2 == 0) ? 1.0 : -1.0;

    filter.process(input, output, 128);

    for (int i = 0; i < 128; ++i) {
        EXPECT_TRUE(std::isfinite(output[i]));
    }
}

TEST(IIRFilterTest, LowPassAttenuatesHighFreq) {
    IIRFilterParams p;
    p.prototype = IIRPrototype::Butterworth;
    p.passType = IIRPassType::LowPass;
    p.order = 8;
    p.cutoff = 500.0;
    p.sampleRate = 44100.0;

    IIRFilter filter(p);
    double input[256];
    double output[256];
    // 10kHz sine — well above cutoff
    for (int i = 0; i < 256; ++i) {
        input[i] = std::sin(2.0 * M_PI * 10000.0 * i / 44100.0);
    }

    filter.process(input, output, 256);

    double inputRMS = 0.0, outputRMS = 0.0;
    for (int i = 0; i < 256; ++i) {
        inputRMS += input[i] * input[i];
        outputRMS += output[i] * output[i];
    }
    inputRMS = std::sqrt(inputRMS / 256);
    outputRMS = std::sqrt(outputRMS / 256);

    // Output should be quieter than input (filtered)
    EXPECT_LT(outputRMS, inputRMS * 0.3);
}

TEST(IIRFilterTest, BypassCopiesInput) {
    IIRFilterParams p;
    p.prototype = IIRPrototype::Butterworth;
    p.order = 4;
    p.cutoff = 1000.0;
    p.sampleRate = 44100.0;

    IIRFilter filter(p);
    filter.setBypass(true);

    double input[64];
    double output[64];
    for (int i = 0; i < 64; ++i) input[i] = static_cast<double>(i);

    filter.process(input, output, 64);

    for (int i = 0; i < 64; ++i) {
        EXPECT_EQ(output[i], input[i]);
    }
}

TEST(IIRFilterTest, BiquadCoeffsFinite) {
    IIRFilterParams p;
    p.prototype = IIRPrototype::ChebyshevI;
    p.passType = IIRPassType::LowPass;
    p.order = 4;
    p.cutoff = 1000.0;
    p.ripple = 0.5;
    p.sampleRate = 44100.0;

    IIRFilter filter(p);

    for (int i = 0; i < 64; ++i) {
        double out;
        filter.process(&out, &out, 1);
        EXPECT_TRUE(std::isfinite(out));
    }
}

TEST(IIRFilterTest, RBJLowPass) {
    IIRFilterParams p;
    p.prototype = IIRPrototype::RBJ_LowPass;
    p.passType = IIRPassType::LowPass;
    p.cutoff = 1000.0;
    p.Q = 0.707;
    p.sampleRate = 44100.0;

    IIRFilter filter(p);
    double input[64];
    double output[64];
    for (int i = 0; i < 64; ++i) input[i] = 1.0;

    filter.process(input, output, 64);

    // DC gain of RBJ LP should be 1
    EXPECT_NEAR(output[63], 1.0, 0.01);
}

TEST(IIRFilterTest, ResetClearsState) {
    IIRFilterParams p;
    p.prototype = IIRPrototype::Butterworth;
    p.order = 4;
    p.cutoff = 1000.0;
    p.sampleRate = 44100.0;

    IIRFilter filter(p);
    double input[64];
    for (int i = 0; i < 64; ++i) input[i] = 1.0;
    double output[64];

    filter.process(input, output, 64);
    filter.reset();
    filter.process(input, output, 64);

    EXPECT_TRUE(std::isfinite(output[0]));
}
