#pragma once

#include <cstdint>

namespace Meta {
    class AbstractTime {
    public:
        virtual uint32_t now(void) {
            return 0; // TODO: Default to ctime
        }

        virtual void delayMs(uint32_t ms) {
            uint32_t start = now();
            while (now() - start < ms) {}
        }
    };
}