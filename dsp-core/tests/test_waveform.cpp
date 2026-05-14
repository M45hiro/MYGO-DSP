#include <gtest/gtest.h>
#include "WaveformGenerator.h"

using namespace mygo_dsp;

TEST(WaveformGeneratorTest, SineWaveOutput) {
    WaveformParams p;
    p.type = WaveformType::Sine;
    p.frequency = 440.0;
    p.amplitude = 0.5;
    p.sampleRate = 44100.0;

    WaveformGenerator gen(p);
    double output[128];
    gen.process(nullptr, output, 128);

    EXPECT_NEAR(output[0], 0.0, 1e-6);
    EXPECT_GT(output[1], 0.0);
    EXPECT_GT(output[10], output[5]);
}

TEST(WaveformGeneratorTest, SineFrequency) {
    WaveformParams p;
    p.type = WaveformType::Sine;
    p.frequency = 1000.0;
    p.amplitude = 1.0;
    p.sampleRate = 10000.0;

    WaveformGenerator gen(p);
    double output[100];
    gen.process(nullptr, output, 100);

    double maxVal = 0.0;
    for (int i = 0; i < 100; ++i) {
        if (std::abs(output[i]) > maxVal) maxVal = std::abs(output[i]);
    }
    EXPECT_NEAR(maxVal, 1.0, 0.05);
}

TEST(WaveformGeneratorTest, BypassZerosOutput) {
    WaveformParams p;
    p.type = WaveformType::Sine;
    p.amplitude = 0.5;
    WaveformGenerator gen(p);
    gen.setBypass(true);

    double input[64] = {0.0};
    double output[64];
    gen.process(nullptr, output, 64);

    for (int i = 0; i < 64; ++i) {
        EXPECT_EQ(output[i], 0.0);
    }
}

TEST(WaveformGeneratorTest, SquareWaveDutyCycle) {
    WaveformParams p;
    p.type = WaveformType::Square;
    p.frequency = 100.0;
    p.amplitude = 1.0;
    p.dutyCycle = 0.25;
    p.sampleRate = 10000.0;

    WaveformGenerator gen(p);
    double output[200];
    gen.process(nullptr, output, 200);

    int positive = 0, negative = 0;
    for (int i = 0; i < 200; ++i) {
        if (output[i] > 0.5) ++positive;
        else if (output[i] < -0.5) ++negative;
    }
    EXPECT_GT(positive, 0);
    EXPECT_GT(negative, 0);
}

TEST(WaveformGeneratorTest, WhiteNoiseNonZero) {
    WaveformParams p;
    p.type = WaveformType::WhiteNoise;
    p.amplitude = 1.0;
    WaveformGenerator gen(p);

    double output[256];
    gen.process(nullptr, output, 256);

    double sum = 0.0;
    for (int i = 0; i < 256; ++i) sum += std::abs(output[i]);
    EXPECT_GT(sum, 0.0);
}

TEST(WaveformGeneratorTest, ResetPhase) {
    WaveformParams p;
    p.type = WaveformType::Sine;
    p.frequency = 440.0;
    p.amplitude = 1.0;
    p.sampleRate = 44100.0;

    WaveformGenerator gen(p);
    double buf1[64], buf2[64];
    gen.process(nullptr, buf1, 64);
    gen.reset();
    gen.process(nullptr, buf2, 64);

    for (int i = 0; i < 64; ++i) {
        EXPECT_NEAR(buf1[i], buf2[i], 1e-10);
    }
}
