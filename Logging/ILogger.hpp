#pragma once

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <array>
#if !defined(PLATFORM_ID)
#include <iostream>
#include <ctime>
#endif

namespace Meta {
    typedef enum {
        LOG_LEVEL_ERROR = 0,
        LOG_LEVEL_WARNING = 1,
        LOG_LEVEL_INFO = 2,
        LOG_LEVEL_DEBUG = 3,
    } LogLevel;

    /**
     * @brief An abstract logger wrapper for interfacing with a given system's
     *         logging system.
     */
    class ILogger {
    private:
        LogLevel level;

    protected:
        virtual const char* getTimestamp() = 0;

        static char levelTag(LogLevel level) {
            switch (level) {
                case LOG_LEVEL_ERROR:   return 'E';
                case LOG_LEVEL_WARNING: return 'W';
                case LOG_LEVEL_INFO:    return 'I';
                case LOG_LEVEL_DEBUG:   return 'D';
            }
            return '?';
        }

    public:
        ILogger(LogLevel level) : level(level) {}

        virtual ~ILogger() = default;

        virtual void rawLog(const char* msg) = 0;

        LogLevel getLevel() const {
            return level;
        }

        void setLevel(LogLevel l) {
            level = l;
        }

        virtual void vlog(LogLevel level, const char* msg, const char* file, size_t line, va_list args) {
            char buffer[1024];
            int written = snprintf(buffer, sizeof(buffer),
                                   "[%s] %c: %s:%lu > ",
                                   getTimestamp(),
                                   levelTag(level),
                                   file,
                                   (unsigned long)line);
            if (written < 0) {
                return;
            }
            size_t used = (static_cast<size_t>(written) < sizeof(buffer))
                            ? static_cast<size_t>(written)
                            : sizeof(buffer) - 1;
            int body = vsnprintf(buffer + used, sizeof(buffer) - used, msg, args);
            if (body > 0) {
                used += (static_cast<size_t>(body) < sizeof(buffer) - used)
                          ? static_cast<size_t>(body)
                          : sizeof(buffer) - used - 1;
            }
            if (used < sizeof(buffer) - 1) {
                buffer[used++] = '\n';
            }
            buffer[used] = '\0';
            rawLog(buffer);
        }

        void log(LogLevel level, const char* msg, const char* file, size_t line, ...) {
            if (level > this->level) return;
            va_list args;
            va_start(args, line);
            vlog(level, msg, file, line, args);
            va_end(args);
        }

        template<typename T, size_t N>
        void log_hexdump(const std::array<T, N>& arr, const char* /*file*/, size_t /*line*/, LogLevel /*level*/ = LOG_LEVEL_INFO) {
            size_t bytesWritten = 0;
            for (auto t : arr) {
                for (size_t i = 0; i < sizeof(T); i++) {
                    char hexStr[3];
                    snprintf(hexStr, sizeof(hexStr), "%02X", (uint8_t)(t >> (i * 8)));
                    this->rawLog(hexStr);
                    this->rawLog(" ");
                    bytesWritten++;

                    if (bytesWritten % 16 == 0) {
                        this->rawLog("\n");
                    }
                    else if (bytesWritten % 8 == 0) {
                        this->rawLog(" ");
                    }
                }
            }
        }

    };

#if !defined(PLATFORM_ID)
    class StdLogger : public ILogger {
    private:
        const char* getTimestamp() override {
            time_t now = time(nullptr);
            struct tm * timeinfo = localtime(&now);
            static char buffer[80];
            strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
            return buffer;
        }
    public:
        StdLogger(LogLevel level = LOG_LEVEL_DEBUG) : ILogger(level) {}
        void rawLog(const char* msg) override {
            std::cout << msg;
        }
    };
#else
    class NullLogger : public ILogger {
    private:
        const char* getTimestamp() override { return ""; }
    public:
        NullLogger() : ILogger(LOG_LEVEL_ERROR) {}
        void rawLog(const char*) override {}
    };
#endif

    // If you find youself using this class directly, you're bad and you should feel bad.
    class LogBroker {
    private:
        static inline ILogger* logger;

    public:
        template<typename T>
        static inline void setLogger(T* l = nullptr) {
            if (l) {
                logger = l;
                return;
            }
#if defined(PLATFORM_ID)
            static NullLogger defaultLogger;
#else
            static StdLogger defaultLogger(LOG_LEVEL_DEBUG);
#endif
            logger = &defaultLogger;
        }

        static inline ILogger* getLogger() {
            if (logger == nullptr) {
#if defined(PLATFORM_ID)
                static NullLogger defaultLogger;
#else
                static StdLogger defaultLogger(LOG_LEVEL_DEBUG);
#endif
                logger = &defaultLogger;
            }
            return logger;
        }
    };

    #define SET_LOGGER(logger) Meta::LogBroker::setLogger(logger)

    /**
     * @brief The current logging level
     */
    #define LOG_LEVEL LogBroker::getLogger()->getLevel()
    #define SET_LOG_LEVEL(level) LogBroker::getLogger()->setLevel(level)

    /**
     * @brief Enter text into the log without headers, newlines, or carriage returns.
     */
    #define RAW_LOG(msg) LogBroker::getLogger()->rawLog(msg)

    /**
     * @brief Strip the path from file names
     */
    #define SIMPLIFY_F_NAME(fpath) strrchr(fpath, '/') ? strrchr(fpath, '/') + 1 : fpath

    #define LOG(level, msg, ...)  Meta::LogBroker::getLogger()->log(level, msg, SIMPLIFY_F_NAME(__FILE__), __LINE__, ##__VA_ARGS__)
    #define LOG_ERROR(msg, ...)   Meta::LogBroker::getLogger()->log(Meta::LOG_LEVEL_ERROR, msg, SIMPLIFY_F_NAME(__FILE__), __LINE__, ##__VA_ARGS__)
    #define LOG_WARNING(msg, ...) Meta::LogBroker::getLogger()->log(Meta::LOG_LEVEL_WARNING, msg, SIMPLIFY_F_NAME(__FILE__), __LINE__, ##__VA_ARGS__)
    #define LOG_INFO(msg, ...)    Meta::LogBroker::getLogger()->log(Meta::LOG_LEVEL_INFO, msg, SIMPLIFY_F_NAME(__FILE__), __LINE__, ##__VA_ARGS__)
    #define LOG_DEBUG(msg, ...)   Meta::LogBroker::getLogger()->log(Meta::LOG_LEVEL_DEBUG, msg, SIMPLIFY_F_NAME(__FILE__), __LINE__, ##__VA_ARGS__)

}
