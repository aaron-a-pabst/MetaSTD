#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <cstring>

namespace Meta {
    typedef struct {
        size_t errorCode;
        const char* errorName;
        const char* file;
    } ErrorDef;

    template<size_t N>
    class Errors {
    private:
        static std::array<ErrorDef, N> errors;

    public:
        static constexpr const ErrorDef& registerError(const char* errorName, const char* file) {
            ErrorDef error = {N, errorName, file};
            errors[N] = error;
            return errors[N];
        }
    };

    // Define the static member variable
    template<size_t N>
    std::array<ErrorDef, N> Errors<N>::errors;

    constexpr const char* simplifyFileName(const char* file) {   
        return strrchr(file, '/') ? strrchr(file, '/') + 1 : file;
    }

    constexpr ErrorDef registerError(const char* errorName, const char* file = __FILE__) {
        return Errors<__COUNTER__>::registerError(errorName, simplifyFileName(file));
    }

    /**
    * @brief Global error registry
    */
    static constexpr auto errors = Errors<__COUNTER__>();

    // Error Definition
    typedef struct {
        ErrorDef errorDef;
        const char* msg;
        size_t line;
    } Error;

    inline bool operator==(const Error& lhs, const ErrorDef& rhs) {
        return lhs.errorDef.errorCode == rhs.errorCode;
    }

    inline bool operator==(const ErrorDef& lhs, const Error& rhs) {
        return rhs == lhs;
    }
}
  