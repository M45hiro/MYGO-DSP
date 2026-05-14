#pragma once
#include "WaveformGenerator.h"
#include "FIRFilter.h"
#include "IIRFilter.h"
#include <memory>
#include <unordered_map>
#include <mutex>
#include <vector>

namespace mygo_dsp {

class Engine {
public:
    static Engine& instance();

    void init(double sampleRate, size_t bufferSize);

    int addWave(const WaveformParams& params);
    void removeWave(int id);
    void updateWave(int id, const WaveformParams& params);

    int addFilterFIR(const FIRFilterParams& params);
    int addFilterIIR(const IIRFilterParams& params);
    void removeFilter(int id);
    void updateFilter(int id, const FIRFilterParams& params);
    void updateFilter(int id, const IIRFilterParams& params);
    void setWaveBypass(int id, bool bypass);
    void setFilterBypass(int id, bool bypass);

    const std::vector<double>& processAudio();

    int getWaveCount() const;
    int getFilterCount() const;
    size_t getBufferSize() const;

    bool getFilterPoleZero(int id,
        std::vector<std::complex<double>>& poles,
        std::vector<std::complex<double>>& zeros) const;
    bool getFilterZeros(int id,
        std::vector<std::complex<double>>& zeros) const;

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

private:
    Engine() = default;

    mutable std::mutex mutex_;
    double sampleRate_ = 48000.0;
    size_t bufferSize_ = 256;
    int nextWaveId_ = 0;
    int nextFilterId_ = 0;
    std::unordered_map<int, std::unique_ptr<WaveformGenerator>> waves_;
    std::unordered_map<int, std::unique_ptr<IOperator>> filters_;
    std::vector<double> outputBuffer_;
    std::vector<double> tempBuffer_;
    std::vector<double> tempBuffer2_;
};

} // namespace mygo_dsp
