#pragma once
#ifndef IOPERATOR_H
#define IOPERATOR_H

#include <cstddef>

namespace mygo_dsp {

class IOperator {
public:
    virtual ~IOperator() = default;

    virtual void process(const double* input, double* output, size_t numSamples) = 0;

    virtual void reset() = 0;

    virtual bool isBypassed() const = 0;

    virtual void setBypass(bool bypass) = 0;
};

} // namespace mygo_dsp

#endif // IOPERATOR_H
