#include <gtest/gtest.h>
#include "FIRFilter.h"

using namespace mygo_dsp;

TEST(FIRFilterTest, LowPassPreservesDC) {
    FIRFilterParams p;
    p.passType = FilterPassType::LowPass;
    p.cutoff = 1000.0;
    p.sampleRate = 44100.0;
    p.order = 64;
    p.window = WindowType::Hamming;

    FIRFilter filter(p);
    double input[128];
    double output[128];
    for (int i = 0; i < 128; ++i) input[i] = 1.0;

    filter.process(input, output, 128);

    // DC should pass
    EXPECT_NEAR(output[64], 1.0, 0.15);
}

TEST(FIRFilterTest, HighPassBlocksDC) {
    FIRFilterParams p;
    p.passType = FilterPassType::HighPass;
    p.cutoff = 1000.0;
    p.sampleRate = 44100.0;
    p.order = 64;
    p.window = WindowType::Hamming;

    FIRFilter filter(p);
    double input[128];
    double output[128];
    for (int i = 0; i < 128; ++i) input[i] = 1.0;

    filter.process(input, output, 128);

    // DC should be blocked (near zero)
    EXPECT_NEAR(output[64], 0.0, 0.05);
}

TEST(FIRFilterTest, ImpulseResponseLength) {
    FIRFilterParams p;
    p.order = 32;
    p.cutoff = 1000.0;
    p.sampleRate = 44100.0;
    p.window = WindowType::Rectangular;

    FIRFilter filter(p);
    double input[64] = {0.0};
    input[0] = 1.0;
    double output[64];
    filter.process(input, output, 64);

    // Impulse response should have non-zero values
    double sum = 0.0;
    for (int i = 0; i < 64; ++i) sum += std::abs(output[i]);
    EXPECT_GT(sum, 0.0);
}

TEST(FIRFilterTest, BypassCopiesInput) {
    FIRFilterParams p;
    p.order = 64;
    FIRFilter filter(p);
    filter.setBypass(true);

    double input[32];
    double output[32];
    for (int i = 0; i < 32; ++i) input[i] = static_cast<double>(i);

    filter.process(input, output, 32);

    for (int i = 0; i < 32; ++i) {
        EXPECT_EQ(output[i], input[i]);
    }
}

TEST(FIRFilterTest, KaiserWindow) {
    FIRFilterParams p;
    p.passType = FilterPassType::LowPass;
    p.cutoff = 1000.0;
    p.sampleRate = 44100.0;
    p.order = 64;
    p.window = WindowType::Kaiser;
    p.kaiserBeta = 4.0;

    FIRFilter filter(p);
    double input[128];
    double output[128];
    for (int i = 0; i < 128; ++i) input[i] = (i < 64) ? 1.0 : 0.0;

    filter.process(input, output, 128);

    EXPECT_TRUE(std::isfinite(output[0]));
}
