#pragma once
#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <mutex>
#include <cstring>
#include <algorithm>
#include <stdexcept>

namespace mygo_dsp {

template<typename T>
class RingBuffer {
public:
    explicit RingBuffer(size_t capacity)
        : buffer_(capacity + 1, T(0))
        , capacity_(capacity + 1)
        , head_(0)
        , tail_(0)
    {}

    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;

    RingBuffer(RingBuffer&& other) noexcept
        : buffer_(std::move(other.buffer_))
        , capacity_(other.capacity_)
        , head_(other.head_.load())
        , tail_(other.tail_.load())
    {
        other.capacity_ = 0;
        other.head_.store(0);
        other.tail_.store(0);
    }

    RingBuffer& operator=(RingBuffer&& other) noexcept {
        if (this != &other) {
            std::lock_guard<std::mutex> lock(mutex_);
            buffer_ = std::move(other.buffer_);
            capacity_ = other.capacity_;
            head_.store(other.head_.load());
            tail_.store(other.tail_.load());
            other.capacity_ = 0;
            other.head_.store(0);
            other.tail_.store(0);
        }
        return *this;
    }

    bool push(const T* data, size_t count) {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t available = capacity_ - 1 - size_unsafe();
        if (count > available) return false;
        for (size_t i = 0; i < count; ++i) {
            buffer_[head_] = data[i];
            head_ = (head_ + 1) % capacity_;
        }
        return true;
    }

    bool push(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t available = capacity_ - 1 - size_unsafe();
        if (available == 0) return false;
        buffer_[head_] = value;
        head_ = (head_ + 1) % capacity_;
        return true;
    }

    size_t pop(T* data, size_t count) {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t sz = size_unsafe();
        size_t toPop = std::min(count, sz);
        for (size_t i = 0; i < toPop; ++i) {
            data[i] = buffer_[tail_];
            tail_ = (tail_ + 1) % capacity_;
        }
        return toPop;
    }

    T pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (size_unsafe() == 0) {
            throw std::runtime_error("RingBuffer underflow");
        }
        T val = buffer_[tail_];
        tail_ = (tail_ + 1) % capacity_;
        return val;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return size_unsafe();
    }

    size_t capacity() const {
        return capacity_ - 1;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return head_.load() == tail_.load();
    }

    bool full() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return ((head_.load() + 1) % capacity_) == tail_.load();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        head_.store(0);
        tail_.store(0);
    }

    void resize(size_t newCapacity) {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t sz = size_unsafe();
        std::vector<T> newBuf(newCapacity + 1, T(0));
        for (size_t i = 0; i < sz; ++i) {
            newBuf[i] = buffer_[(tail_.load() + i) % capacity_];
        }
        buffer_ = std::move(newBuf);
        capacity_ = newCapacity + 1;
        head_.store(sz);
        tail_.store(0);
    }

private:
    size_t size_unsafe() const {
        size_t h = head_.load();
        size_t t = tail_.load();
        if (h >= t) return h - t;
        return capacity_ - t + h;
    }

    std::vector<T> buffer_;
    size_t capacity_;
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
    mutable std::mutex mutex_;
};

} // namespace mygo_dsp

#endif // BUFFER_H
