#pragma once

namespace Harbor {
    /**
     * @brief Thin lock wrapper.
     */
    class AbstractLock {
    public:
        virtual void lock() = 0;
        virtual void unlock() = 0;
    };
}