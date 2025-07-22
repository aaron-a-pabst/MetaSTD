#pragma once

#include <cstdint>

namespace Meta {
    class AbstractTime {
    public:
        virtual uint32_t now(void) {
            return 0; // TODO: Default to ctime
        }
    };
}