#pragma once

#include <array>
#include <cstdlib>
#include <limits>
#include <cstdint>

namespace Meta {
    template<typename T, size_t N, size_t seed>
    static std::array<T, N> randomArray() {
        std::srand(seed);
        std::array<T, N> arr;
        for (size_t i = 0; i < N; i++) {
            arr[i] = std::rand() % std::numeric_limits<T>::max();
        }
        return arr;
    }

    template<typename T, size_t N>
    static constexpr std::array<T, N> range() {
        std::array<T, N> arr;
        for (size_t i = 0; i < N; i++) {
            arr[i] = i % std::numeric_limits<T>::max();
        }
        return arr;
    }

    template<typename T>
    static constexpr std::array<uint8_t, sizeof(T)> getBytes(const T& t) {
        std::array<uint8_t, sizeof(T)> bytes;
        for (size_t i = 0; i < sizeof(T); i++) {
            bytes[i] = (t >> (i * 8)) & 0xFF;
        }
        return bytes;
    }

    template<typename T, size_t N>
    static std::array<uint8_t, N * sizeof(T)> toByteArray(const std::array<T, N>& arr) {
        std::array<uint8_t, N * sizeof(T)> bytes;
        size_t bIdx = 0;
        for (auto t : arr) {
            auto elementBytes = getBytes(t);
            std::copy(elementBytes.begin(), elementBytes.end(), bytes.begin() + bIdx);
            bIdx += elementBytes.size();
        }
        return bytes;
    }
}

