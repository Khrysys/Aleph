/**
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <mutex>

namespace aleph::platform {
    inline void loggingInit() noexcept {
        static std::once_flag flag;
        std::call_once(flag, []() {
            absl::InitializeLog();
            detail::logging_ready.store(true);
        });
    }

    inline bool isLoggingReady() noexcept { return detail::logging_ready.load(std::memory_order_relaxed); }
}  // namespace aleph::platform