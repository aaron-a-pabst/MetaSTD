#pragma once

#include <array>
#include <initializer_list>
#include <cstdint>
#include "../Errors/ErrorUnion.hpp"
#include "../Errors/Errors.hpp"
#include "../Logging/ILogger.hpp"
#include "ArrayUtils.hpp"

namespace Meta {

const ErrorDef BUFFER_ERROR_OVERRUN = registerError("BUFFER_ERROR_OVERRUN");

template<typename T, size_t C>
class Buffer {
private:
    std::array<T, C> data;
    size_t length;

public:
    Buffer() : length(0) {
        data.fill(T());
    }

    template<size_t N>
    Buffer(const std::array<T, N>& arr) : length(N) {
        std::copy(arr.begin(), arr.begin() + N, data.begin());
    }

    Buffer(const T* arr, size_t len) : length(len) {
        std::copy(arr, arr + len, data.begin());
    }

    Buffer(std::initializer_list<T> list) : length(list.size()) {
        std::copy(list.begin(), list.end(), data.begin());
    }

    Buffer(const Buffer<T, C>& other) : length(other.length) {
        std::copy(other.data.begin(), other.data.begin() + other.length, data.begin());
    }

    void clear() {
        length = 0;
    }

    const std::array<T, C>& getRaw() const {
        return data;
    }

    const T* cArr() const {
        return data.data();
    }

    ErrorUnion<void> push_back(T t) {
        if (length >= C) {
            HARBOR_LOG_ERROR("Buffer overrun");
            return ErrorUnion<void>(MAKE_ERROR(BUFFER_ERROR_OVERRUN, "Buffer overrun"));
        }
        data[length++] = t;
        return ErrorUnion<void>();
    }

    ErrorUnion<void> append(const T* arr, size_t len) {
        if (length + len > C) {
            HARBOR_LOG_ERROR("Buffer overrun");
            return ErrorUnion<void>(MAKE_ERROR(BUFFER_ERROR_OVERRUN, "Buffer overrun"));
        }
        std::copy(arr, arr + len, data.begin() + length);
        length += len;
        return ErrorUnion<void>();
    }

    template<size_t N>
    ErrorUnion<void> append(const Buffer<T, N>& other) {
        static_assert(N <= C, "Buffer overrun");
        return append(other.cArr(), other.size());
    }

    T pop_back() {
        return data[length--];
    }

    template<size_t N>
    ErrorUnion<void> copy(const Buffer<T, N>& from, size_t offset = 0, size_t count = (size_t)-1) {
        
        if (offset > from.size()) {
            HARBOR_LOGF(Harbor::LOG_LEVEL_ERROR, "Offset out of bounds: %d > %d", offset, from.size());
            return ErrorUnion<void>(MAKE_ERROR(BUFFER_ERROR_OVERRUN, "Offset out of bounds"));
        }

        if (count == (size_t)-1) {
            count = from.size() - offset;
        }
        std::copy(from.begin() + offset, from.begin() + offset + count, data.begin() + length);
        length += count;
        return ErrorUnion<void>();
    }

    template<size_t N>
    ErrorUnion<void> copyOver(size_t over, Buffer<T, N>& from, size_t offset = 0, size_t count = (size_t) - 1) {
        if (over + count > C) {
            HARBOR_LOG(Harbor::LOG_LEVEL_ERROR, "Buffer overrun");
            return ErrorUnion<void>(MAKE_ERROR(BUFFER_ERROR_OVERRUN, "Buffer overrun"));
        }

        if (count == (size_t)-1) {
            count = from.size() - offset;
        }
        std::copy(from.begin() + offset, from.begin() + offset + count, data.begin() + over);
        return ErrorUnion<void>();
    }

    size_t size() const {
        return length;
    }

    T& operator[](size_t idx) {
        return data[idx];
    }

    const T& operator[](size_t idx) const {
        return data[idx];
    }

    const T* begin() const {
        return data.begin();
    }

    const T* end() const {
        return data.begin() + length;
    }  

    Buffer<uint8_t, C * sizeof(T)> toBytes() const {
        Buffer<uint8_t, C * sizeof(T)> bytes;
        for (size_t i = 0; i < length; i++) {
            for(uint8_t j = 0; j < sizeof(T); j++) {
                bytes.push_back((data[i] >> (j * 8)) & 0xFF);
            }
        }
        return bytes;
    }

    template<size_t start, size_t end>
    Buffer<uint8_t, end - start> subBuffer() const {
        Buffer<uint8_t, end - start> sub;
        for (size_t i = start; i < end; i++) {
            sub.push_back(data[i]);
        }
        return sub;
    }

    /**
     * @brief Take(/remove) n bytes from the front of this buffer.
     */
    template<size_t N>
    Buffer<uint8_t, N> take(size_t n) {
        static_assert(N <= C, "Buffer overrun");
        // Extract the bytes to take
        Buffer<uint8_t, N> taken;
        taken.copy(*this, 0, n);

        // Shift the remaining bytes left
        copyOver(0, *this, n, this->size() - n);
        length -= n;

        return taken;
    }

    void dumpBytes(LogLevel level = LOG_LEVEL_DEBUG) const {
        if (level < logger->getLevel())
            return;
        auto bytes = toBytes();
        size_t bytesWritten = 0;
        for (size_t i = 0; i < bytes.size(); i++) {
            char hexStr[3];
            snprintf(hexStr, sizeof(hexStr), "%02X", bytes[i]);
            logger->rawLog(hexStr);
            logger->rawLog(" ");
            bytesWritten++;

            if (bytesWritten % 16 == 0) {
                logger->rawLog("\n\r");
            }
            else if (bytesWritten % 8 == 0) {
                logger->rawLog(" ");
            }
        }
        logger->rawLog("\n\r");
    }
};

}