#include <gtest/gtest.h>
#include "Engine.h"

using namespace mygo_dsp;

TEST(EngineTest, Singleton) {
    auto& e1 = Engine::instance();
    auto& e2 = Engine::instance();
    EXPECT_EQ(&e1, &e2);
}

TEST(EngineTest, InitAndProcess) {
    auto& engine = Engine::instance();
    engine.init(44100.0, 128);

    WaveformParams p;
    p.type = WaveformType::Sine;
    p.frequency = 440.0;
    p.amplitude = 0.5;
    p.sampleRate = 44100.0;

    int wid = engine.addWave(p);
    EXPECT_GE(wid, 0);

    const auto& data = engine.processAudio();
    EXPECT_EQ(data.size(), 128);
    EXPECT_GT(data[10], 0.0);

    engine.removeWave(wid);
    EXPECT_EQ(engine.getWaveCount(), 0);
}

TEST(EngineTest, FilterChain) {
    auto& engine = Engine::instance();
    engine.init(44100.0, 128);

    WaveformParams wp;
    wp.type = WaveformType::Square;
    wp.frequency = 200.0;
    wp.amplitude = 0.5;
    wp.sampleRate = 44100.0;
    engine.addWave(wp);

    IIRFilterParams fp;
    fp.prototype = IIRPrototype::Butterworth;
    fp.passType = IIRPassType::LowPass;
    fp.order = 4;
    fp.cutoff = 300.0;
    fp.sampleRate = 44100.0;
    int fid = engine.addFilterIIR(fp);

    const auto& filtered = engine.processAudio();

    engine.removeFilter(fid);
    const auto& unfiltered = engine.processAudio();

    double sumF = 0.0, sumU = 0.0;
    for (size_t i = 0; i < 128; ++i) {
        sumF += std::abs(filtered[i]);
        sumU += std::abs(unfiltered[i]);
    }

    // Filtered should have less energy (LP at 300Hz on 200Hz square harmonics)
    EXPECT_LT(sumF, sumU);
}

TEST(EngineTest, WaveBypass) {
    auto& engine = Engine::instance();
    engine.init(44100.0, 128);

    WaveformParams wp;
    wp.type = WaveformType::Sine;
    wp.frequency = 440.0;
    wp.amplitude = 0.5;
    wp.sampleRate = 44100.0;
    int wid = engine.addWave(wp);

    // Wave active: should have output
    const auto& active = engine.processAudio();

    engine.setWaveBypass(wid, true);
    const auto& bypassed = engine.processAudio();

    double sumActive = 0.0, sumBypassed = 0.0;
    for (size_t i = 0; i < 128; ++i) {
        sumActive += std::abs(active[i]);
        sumBypassed += std::abs(bypassed[i]);
    }

    EXPECT_GT(sumActive, 0.0);
    EXPECT_EQ(sumBypassed, 0.0);

    engine.setWaveBypass(wid, false);
}

TEST(EngineTest, MultipleWavesSum) {
    auto& engine = Engine::instance();
    engine.init(44100.0, 128);

    WaveformParams p1, p2;
    p1.type = WaveformType::Sine;
    p1.frequency = 440.0;
    p1.amplitude = 0.5;
    p1.sampleRate = 44100.0;

    p2.type = WaveformType::Sine;
    p2.frequency = 880.0;
    p2.amplitude = 0.3;
    p2.sampleRate = 44100.0;

    engine.addWave(p1);
    engine.addWave(p2);
    EXPECT_EQ(engine.getWaveCount(), 2);

    const auto& data = engine.processAudio();
    EXPECT_GT(data[10], 0.0);
}
