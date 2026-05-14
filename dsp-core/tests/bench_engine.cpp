#include <chrono>
#include <cstdio>
#include <cmath>
#include "Engine.h"
#include "WaveformGenerator.h"
#include "FIRFilter.h"
#include "IIRFilter.h"

using namespace mygo_dsp;
using namespace std::chrono;

class Timer {
    high_resolution_clock::time_point start_;
public:
    void reset() { start_ = high_resolution_clock::now(); }
    double elapsed_ms() const {
        auto end = high_resolution_clock::now();
        return duration_cast<nanoseconds>(end - start_).count() / 1e6;
    }
};

static void bench_waveform_generation(int iterations = 10000) {
    WaveformParams p;
    p.type = WaveformType::Sine;
    p.frequency = 440.0;
    p.amplitude = 1.0;
    p.sampleRate = 48000.0;

    WaveformGenerator gen(p);
    double buffer[512];

    Timer t;
    t.reset();
    for (int i = 0; i < iterations; ++i) {
        gen.process(nullptr, buffer, 512);
    }
    double ms = t.elapsed_ms();

    std::printf("Waveform generation (%d x 512 samples): %.2f ms (%.2f ns/sample)\n",
                iterations, ms, ms * 1e6 / (iterations * 512));
}

static void bench_fir_filter(int order = 128, int iterations = 10000) {
    FIRFilterParams p;
    p.order = order;
    p.cutoff = 1000.0;
    p.sampleRate = 48000.0;
    p.window = WindowType::Hamming;

    FIRFilter filter(p);
    double input[512];
    double output[512];
    for (int i = 0; i < 512; ++i) input[i] = std::sin(2.0 * M_PI * 440.0 * i / 48000.0);

    Timer t;
    t.reset();
    for (int i = 0; i < iterations; ++i) {
        filter.process(input, output, 512);
    }
    double ms = t.elapsed_ms();

    std::printf("FIR filter (order=%d, %d x 512 samples): %.2f ms (%.2f ns/sample)\n",
                order, iterations, ms, ms * 1e6 / (iterations * 512));
}

static void bench_iir_filter(int order = 8, int iterations = 100000) {
    IIRFilterParams p;
    p.prototype = IIRPrototype::Butterworth;
    p.passType = IIRPassType::LowPass;
    p.order = order;
    p.cutoff = 1000.0;
    p.sampleRate = 48000.0;

    IIRFilter filter(p);
    double input[512];
    double output[512];
    for (int i = 0; i < 512; ++i) input[i] = std::sin(2.0 * M_PI * 440.0 * i / 48000.0);

    Timer t;
    t.reset();
    for (int i = 0; i < iterations; ++i) {
        filter.process(input, output, 512);
    }
    double ms = t.elapsed_ms();

    std::printf("IIR filter (order=%d, %d x 512 samples): %.2f ms (%.2f ns/sample)\n",
                order, iterations, ms, ms * 1e6 / (iterations * 512));
}

static void bench_engine_pipeline(int iterations = 100000) {
    auto& engine = Engine::instance();
    engine.init(48000.0, 512);

    WaveformParams wp;
    wp.type = WaveformType::Sine;
    wp.frequency = 440.0;
    wp.amplitude = 0.5;
    wp.sampleRate = 48000.0;
    engine.addWave(wp);

    IIRFilterParams fp;
    fp.prototype = IIRPrototype::Butterworth;
    fp.passType = IIRPassType::LowPass;
    fp.order = 4;
    fp.cutoff = 1000.0;
    fp.sampleRate = 48000.0;
    engine.addFilterIIR(fp);

    Timer t;
    t.reset();
    for (int i = 0; i < iterations; ++i) {
        engine.processAudio();
    }
    double ms = t.elapsed_ms();

    std::printf("Engine pipeline (wave+filter, %d iterations): %.2f ms (%.2f us/call)\n",
                iterations, ms, ms * 1000.0 / iterations);
}

int main() {
    std::printf("=== DSP Performance Benchmarks ===\n\n");

    bench_waveform_generation();

    bench_fir_filter(32);
    bench_fir_filter(64);
    bench_fir_filter(128);

    bench_iir_filter(4);
    bench_iir_filter(8);
    bench_iir_filter(12);

    bench_engine_pipeline();

    std::printf("\n=== Done ===\n");
    return 0;
}
