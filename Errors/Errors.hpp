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

    /**
     * @brief A simple error registery that allows automatically defining errors
     *         at compile-time.
     */
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

    template<size_t N>
    std::array<ErrorDef, N> Errors<N>::errors;

    #define SIMPLIFY_FILE_NAME(file) strrchr(file, '/') ? strrchr(file, '/') + 1 : file

    /**
     * @brief Define an error with a unique code.
     */
    #define REGISTER_ERROR(errorName) Errors<__COUNTER__>::registerError(errorName, SIMPLIFY_FILE_NAME(__FILE__))

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

    /**
     * @brief Create an occurance of a given error def.
     */
    #define MAKE_ERROR(errorDef, msg) Error{errorDef, msg, __LINE__}
}
  