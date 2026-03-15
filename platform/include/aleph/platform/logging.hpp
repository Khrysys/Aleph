#pragma once

#include "compiler.hpp"
#include <absl/log/initialize.h>
#include <absl/log/log.h>

namespace aleph::platform {
    namespace detail {
        /** 
         * True after `loggingInit()` has been called. 
         */
        inline bool logging_ready = false;
    }

    /**
     * Initializes the absl logging subsystem.
     *
     * Must be called before any other engine subsystem initialization.
     * Subsequent calls are no-ops. Cannot fail.
     */
    inline void loggingInit() noexcept;

    /**
     * Returns true if the logging subsystem has been successfully initialized.
     */
    [[nodiscard]] inline bool isLoggingReady() noexcept;

} // namespace aleph::platform

#include "logging.inl"