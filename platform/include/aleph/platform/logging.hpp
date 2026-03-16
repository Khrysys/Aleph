/**
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <absl/log/initialize.h>
#include <absl/log/log.h>

namespace aleph::platform {
    namespace detail {
        /**
         * True after `loggingInit()` has been called.
         */
        inline bool logging_ready = false;
    }  // namespace detail

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
    [[nodiscard]] inline auto isLoggingReady() noexcept -> bool;

}  // namespace aleph::platform

#include "logging.inl"