#include "Engine.h"

namespace mygo_dsp {

Engine& Engine::instance() {
    static Engine engine;
    return engine;
}

void Engine::init(double sampleRate, size_t bufferSize) {
    std::lock_guard<std::mutex> lock(mutex_);
    sampleRate_ = sampleRate;
    bufferSize_ = bufferSize;
    outputBuffer_.resize(bufferSize, 0.0);
    tempBuffer_.resize(bufferSize, 0.0);
    tempBuffer2_.resize(bufferSize, 0.0);
    waves_.clear();
    filters_.clear();
    nextWaveId_ = 0;
    nextFilterId_ = 0;
}

int Engine::addWave(const WaveformParams& params) {
    std::lock_guard<std::mutex> lock(mutex_);
    int id = nextWaveId_++;
    auto wave = std::make_unique<WaveformGenerator>(params);
    wave->setId(id);
    wave->reset();
    waves_[id] = std::move(wave);
    return id;
}

void Engine::removeWave(int id) {
    std::lock_guard<std::mutex> lock(mutex_);
    waves_.erase(id);
}

void Engine::updateWave(int id, const WaveformParams& params) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = waves_.find(id);
    if (it != waves_.end()) {
        it->second->setParams(params);
    }
}

int Engine::addFilterFIR(const FIRFilterParams& params) {
    std::lock_guard<std::mutex> lock(mutex_);
    int id = nextFilterId_++;
    auto filter = std::make_unique<FIRFilter>(params);
    filter->setId(id);
    filter->reset();
    filters_[id] = std::move(filter);
    return id;
}

int Engine::addFilterIIR(const IIRFilterParams& params) {
    std::lock_guard<std::mutex> lock(mutex_);
    int id = nextFilterId_++;
    auto filter = std::make_unique<IIRFilter>(params);
    filter->setId(id);
    filter->reset();
    filters_[id] = std::move(filter);
    return id;
}

void Engine::removeFilter(int id) {
    std::lock_guard<std::mutex> lock(mutex_);
    filters_.erase(id);
}

void Engine::updateFilter(int id, const FIRFilterParams& params) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = filters_.find(id);
    if (it != filters_.end()) {
        auto* fir = dynamic_cast<FIRFilter*>(it->second.get());
        if (fir) {
            fir->design(params);
        }
    }
}

void Engine::updateFilter(int id, const IIRFilterParams& params) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = filters_.find(id);
    if (it != filters_.end()) {
        auto* iir = dynamic_cast<IIRFilter*>(it->second.get());
        if (iir) {
            iir->design(params);
        }
    }
}

void Engine::setWaveBypass(int id, bool bypass) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto wit = waves_.find(id);
    if (wit != waves_.end()) {
        wit->second->setBypass(bypass);
    }
}

void Engine::setFilterBypass(int id, bool bypass) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto fit = filters_.find(id);
    if (fit != filters_.end()) {
        fit->second->setBypass(bypass);
    }
}

const std::vector<double>& Engine::processAudio() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::fill(outputBuffer_.begin(), outputBuffer_.end(), 0.0);

    // Sum all waveform generators into tempBuffer
    std::fill(tempBuffer_.begin(), tempBuffer_.end(), 0.0);
    for (auto& [id, wave] : waves_) {
        wave->process(nullptr, tempBuffer2_.data(), bufferSize_);
        for (size_t i = 0; i < bufferSize_; ++i) {
            tempBuffer_[i] += tempBuffer2_[i];
        }
    }

    // Apply filter chain
    const double* currentInput = tempBuffer_.data();
    double* currentOutput = outputBuffer_.data();

    bool firstFilter = true;
    for (auto& [id, filter] : filters_) {
        if (firstFilter) {
            filter->process(currentInput, outputBuffer_.data(), bufferSize_);
            currentInput = outputBuffer_.data();
            firstFilter = false;
        } else {
            filter->process(currentInput, tempBuffer2_.data(), bufferSize_);
            std::swap(outputBuffer_, tempBuffer2_);
            currentInput = outputBuffer_.data();
        }
    }

    if (firstFilter) {
        // No filters, copy summed waves to output
        std::copy(tempBuffer_.begin(), tempBuffer_.end(), outputBuffer_.begin());
    }

    return outputBuffer_;
}

int Engine::getWaveCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int>(waves_.size());
}

int Engine::getFilterCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int>(filters_.size());
}

bool Engine::getFilterPoleZero(int id,
    std::vector<std::complex<double>>& poles,
    std::vector<std::complex<double>>& zeros) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = filters_.find(id);
    if (it == filters_.end()) return false;
    auto* iir = dynamic_cast<IIRFilter*>(it->second.get());
    if (!iir) return false;
    iir->getPoleZero(poles, zeros);
    return true;
}

} // namespace mygo_dsp
