#pragma once


#include <cstdint>
#include "Errors.hpp"

namespace Meta {
    /**
     * @brief Contains etiher an error or a return value depending on the result
     *         of an operation.
     */
    template<typename T>
    class ErrorUnion {
    private:
        bool isError;
        union {
            T value;
            Meta::Error error;
        };

    public:
        ErrorUnion() : isError(false), value() {}

        ErrorUnion(T value) : isError(false), value(value) {}
        ErrorUnion(Meta::Error error) : isError(true), error(error) {}

        ~ErrorUnion() = default;

        ErrorUnion(const ErrorUnion& other) : isError(other.isError) {
            if (isError) {
                error = other.error;
            } else {
                value = other.value;
            }
        }
        ErrorUnion& operator=(const ErrorUnion& other) {
            isError = other.isError;
            value = other.value;
            error = other.error;
            return *this;
        }

        ErrorUnion(ErrorUnion&& other) noexcept : isError(other.isError), value(move(other.value)), error(other.error) {}
        ErrorUnion& operator=(ErrorUnion&& other) noexcept {
            isError = other.isError;
            value = std::move(other.value);
            error = other.error;
            return *this;
        }

        bool hasError() const { return isError; }
        T getValue() const { return value; }
        Meta::Error getError() const { return error; }
    };

    // Special case for void
    template <>
    class ErrorUnion<void> {
    private:
        bool isError;
        Meta::Error error;

    public:
        ErrorUnion() : isError(false), error(Meta::Error()) {}

        ErrorUnion(Meta::Error error) :  isError(true), error(error) {}

        bool hasError() {
            return isError;
        }

        Meta::Error getError() {
            return error;
        }
    };
}