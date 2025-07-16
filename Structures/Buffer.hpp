#pragma once

#include <array>
#include <initializer_list>
#include <cstdint>
#include "../Errors/ErrorUnion.hpp"
#include "../Errors/Errors.hpp"
#include "../Logging/ILogger.hpp"

namespace Meta {

const ErrorDef BUFFER_ERROR_OVERRUN = REGISTER_ERROR("BUFFER_ERROR_OVERRUN");

/**
 * @brief A static structure that provides many of the conviences associated with std::vector, but without
 *         introducing the need for dynamic allocatations and exceptions. 
 *
 * @tparam T The underlying data type the buffer holds.
 * @tparam C The capacity of the buffer as a count of Tsj.
 */
template<typename T, size_t C>
class Buffer {
private:
    std::array<T, C> data; // The underlying (static) array
    size_t length; // How many Ts are currently stored in the buffer

public:
    /**
     * @brief Instantiate an empty, zeroed out buffer.
     */
    Buffer() : length(0) {
        data.fill(T());
    }

    /**
     * @brief Wrap a buffer over a (copy of a) standard array.
     */
    template<size_t N>
    Buffer(const std::array<T, N>& arr) : length(N) {
        static_assert(N <= C, "Instantiating array may not be larger than the buffer capacity.");
        std::copy(arr.begin(), arr.begin() + N, data.begin());
    }

    /**
     * @brief Wrap a buffer around a (copy of a ) C-style array. 
     */
    Buffer(const T* arr, size_t len) : length(len) {
        std::copy(arr, arr + len, data.begin());
    }

    /**
     * @brief Instantiate a buffer from an initializer.
     */
    Buffer(std::initializer_list<T> list) : length(list.size()) {
        std::copy(list.begin(), list.end(), data.begin());
    }

    // Copy
    Buffer(const Buffer<T, C>& other) : length(other.length) {
        std::copy(other.data.begin(), other.data.begin() + other.length, data.begin());
    }

    // Move
    Buffer(Buffer<T, C>&& other) noexcept : data(other.data), length(other.length) {
        other.length = 0;
    }

    // Assignment
    Buffer<T, C>& operator=(const Buffer<T, C>& other) {
        data = other.data;
        length = other.length;
        return *this;
    }

    // Move assignment
    Buffer<T, C>& operator=(Buffer<T, C>&& other) noexcept {
        data = std::move(other.data);
        length = other.length;
        other.length = 0;
        return *this;
    }
    

    /**
     * @brief Reset a buffer to its default state.
     */
    void clear() {
        length = 0;
    }

    /**
     * @brief Get a reference to the underlying array.
     */
    const std::array<T, C>& getRaw() const {
        return data;
    }

    /**
     * @brief Get a raw (read-only) pointer to the buffer's data.
     */
    const T* cArr() const {
        return data.data();
    }

    /**
     * @brief Add a single T to the back of the buffer.
     *
     * @return An error if the buffer's capacity would be exceeded, void otherwise.
     */
    ErrorUnion<void> push_back(T t) {
        if (length >= C) {
            LOG_ERROR("Buffer overrun");
            return ErrorUnion<void>(MAKE_ERROR(BUFFER_ERROR_OVERRUN, "Buffer overrun"));
        }
        data[length++] = t;
        return ErrorUnion<void>();
    }

    /**
     * @brief Add a range of Ts represented as a C-style array.
     *
     * @return An error if the buffer's capacity would be exceeded, void otherwise.
     */
    ErrorUnion<void> append(const T* arr, size_t len) {
        if (length + len > C) {
            LOG_ERROR("Buffer overrun");
            return ErrorUnion<void>(MAKE_ERROR(BUFFER_ERROR_OVERRUN, "Buffer overrun"));
        }
        std::copy(arr, arr + len, data.begin() + length);
        length += len;
        return ErrorUnion<void>();
    }

    template<size_t N>
    ErrorUnion<void> append(const std::array<T, N>& arr) {
        static_assert(N <= C, "Buffer would overrun");
        return append(arr.data(), arr.size());
    }

    /**
     * @brief Add a range of T's depicted by another buffer.
     */
    template<size_t N>
    ErrorUnion<void> append(const Buffer<T, N>& other) {
        static_assert(N <= C, "Buffer overrun");
        return append(other.cArr(), other.size());
    }

    /**
     * @brief Extract a T from the buffer's back.
     */
    T pop_back() {
        return data[length--];
    }

    /**
     * @brief Copy a range of T's into this buffer from another one.
     *
     * @param from A reference to the T source.
     * @param offset Where in from to start copying.
     * @param count How many T's to copy.
     * @return An error if bounds or capacities are violated. Void otherwise.
     */
    template<size_t N>
    ErrorUnion<void> copy(const Buffer<T, N>& from, size_t offset = 0, size_t count = (size_t)-1) {
        if (offset > from.size()) {
            LOG_ERROR("Offset out of bounds: %d > %d", offset, from.size());
            return ErrorUnion<void>(MAKE_ERROR(BUFFER_ERROR_OVERRUN, "Offset out of bounds"));
        }

        if (count == (size_t)-1) {
            count = from.size() - offset;
        }
        std::copy(from.begin() + offset, from.begin() + offset + count, data.begin() + length);
        length += count;
        return ErrorUnion<void>();
    }

    /**
     * @brief Copy a range of T's over the existing buffer contents, possibly overlapping the current back and altering the size.
     */
    template<size_t N>
    ErrorUnion<void> copyOver(size_t over, Buffer<T, N>& from, size_t offset = 0, size_t count = (size_t) - 1) {
        if (over + count > C) {
            LOG_ERROR("Buffer overrun");
            return ErrorUnion<void>(MAKE_ERROR(BUFFER_ERROR_OVERRUN, "Buffer overrun"));
        }

        if (count == (size_t)-1) {
            count = from.size() - offset;
        }
        std::copy(from.begin() + offset, from.begin() + offset + count, data.begin() + over);
        // Update length, accounting for overlap with existing data
        if (over + count > length) {
            length += count - offset;
        }
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

    /**
     * @brief Convert this buffer's contents into a byte string.
     */
    Buffer<uint8_t, C * sizeof(T)> toBytes() const {
        Buffer<uint8_t, C * sizeof(T)> bytes;
        for (size_t i = 0; i < length; i++) {
            bytes.append(getBytes(data[i]));
        }
        return bytes;
    }

    /**
     * @brief Extract a statically defined subset of this buffer. 
     */
    template<size_t start, size_t end>
    Buffer<uint8_t, end - start> subBuffer() const {
        Buffer<uint8_t, end - start> sub;
        for (size_t i = start; i < end; i++) {
            sub.push_back(data[i]);
        }
        return sub;
    }

    /**
     * @brief Take(remove) n bytes from the front of this buffer.
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

    /**
     * @brief Write the contents of this buffer to the local logging system in hexadecimal.
     */
    void hexDump(LogLevel level = LOG_LEVEL_DEBUG, const char* msg = "") const {
        if (level < LOG_LEVEL)
            return;

        LOG(level, msg);
        RAW_LOG("\r\n");

        size_t bytesWritten = 0;
        for (auto b : toBytes()) {
            char hexStr[3];
            snprintf(hexStr, sizeof(hexStr), "%02X", b);
            RAW_LOG(hexStr);
            RAW_LOG(" ");
            bytesWritten++;

            if (bytesWritten % 16 == 0) {
                RAW_LOG("\n\r");
            }
            else if (bytesWritten % 8 == 0) {
                RAW_LOG(" ");
            }
        }
        RAW_LOG("\n\r");
    }
};

}