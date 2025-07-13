#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <array>

namespace Meta {
    typedef enum {
        LOG_LEVEL_ERROR = 0,
        LOG_LEVEL_WARNING = 1,
        LOG_LEVEL_INFO = 2,
        LOG_LEVEL_DEBUG = 3,
    } LogLevel;

    class ILogger {
    private:
        virtual const char* getTimestamp() = 0;

        LogLevel level;

    public:
        ILogger(LogLevel level) : level(level) {}

        virtual void rawLog(const char* msg) = 0;

        LogLevel getLevel() const {
            return level;
        }

        void writeLevel(LogLevel level) {
            switch (level) {
                case LOG_LEVEL_DEBUG:
                    rawLog("[DEBUG]:");
                    break;
                case LOG_LEVEL_INFO:
                    rawLog("[INFO]:");
                    break;
                case LOG_LEVEL_WARNING:
                    rawLog("[WARNING]:");
                    break;
                case LOG_LEVEL_ERROR:
                    rawLog("[ERROR]:");
                    break;
            }
        }

        void log(LogLevel level, const char* msg, const char* file, size_t line) {
            if (level < this->level) 
                return;
            
            // Log level string
            writeLevel(level);

            // Log timestamp
            this->rawLog(getTimestamp());

            this->rawLog(":");

            this->rawLog(file);
            this->rawLog(":");
            
            char lineStr[10];
            snprintf(lineStr, sizeof(lineStr), "%zu", line);
            this->rawLog(lineStr);
            
            this->rawLog(": ");

            // Log message
            this->rawLog(msg);
            
            this->rawLog("\n\r");
        }

        // printf style log
        template<typename... Args>
        void log(LogLevel level, const char* msg, const char* file, size_t line, Args... args) {
            if (level < this->level) return;
            
            // Log level string
            writeLevel(level);

            this->rawLog(getTimestamp());
            this->rawLog(":");
            
            this->rawLog(file);
            this->rawLog(": ");
            
            char lineStr[10];
            snprintf(lineStr, sizeof(lineStr), "%zu", line);
            this->rawLog(lineStr);
            
            // Parse args
            char buffer[1024];
            snprintf(buffer, sizeof(buffer), msg, args...);
            this->rawLog(buffer);
            
            this->rawLog("\n\r");
        }

        template<typename T, size_t N>
        void log_hexdump(const std::array<T, N>& arr, const char* file, size_t line, LogLevel level = LOG_LEVEL_INFO) {
            size_t bytesWritten = 0;
            for (auto t : arr) {
                for (size_t i = 0; i < sizeof(T); i++) {
                    char hexStr[3];
                    snprintf(hexStr, sizeof(hexStr), "%02X", (uint8_t)(t >> (i * 8)));
                    this->rawLog(hexStr);
                    this->rawLog(" ");
                    bytesWritten++;

                    if (bytesWritten % 16 == 0) {
                        this->rawLog("\n\r");
                    }
                    else if (bytesWritten % 8 == 0) {
                        this->rawLog(" ");
                    }
                }
            }
        }
            
    };

    static ILogger* logger;
    constexpr void setLogger(ILogger* l) {
        logger = l;
    }
}
