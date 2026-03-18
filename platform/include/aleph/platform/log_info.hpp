/**
 * @file include/aleph/platform/log_info.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <spdlog/spdlog.h>

#include "intrinsics.hpp"

namespace aleph::platform {

    namespace detail {

        /** Column width for flag name alignment in the diagnostic output. */
        constexpr int LOG_WIDTH = 36;

        /** Log a flag from a bool via spdlog in the diagnostic output. */
        inline auto log_flag(const char* name, bool value) -> void {
            spdlog::info("{:<{}} | {}", name, LOG_WIDTH, value ? "Yes" : "No");
        }

        /** Log a new print section from a bool via spdlog in the diagnostic output. */
        inline auto log_section(const char* title) -> void {
            spdlog::info("");
            spdlog::info("{}", title);
            spdlog::info("{}", std::string(LOG_WIDTH, '-'));
        }

    }  // namespace detail

    /**
     * Logs CPU intrinsics availability.
     * Must be called after `loggingInit()`.
     */
    inline auto logIntrinsicsConfig() -> void {
        detail::log_section("CPU Intrinsics Configuration");

        detail::log_flag("x86intrin.h",
#ifdef ALEPH_HAS_X86INTRIN_H
                         true
#else
                         false
#endif
        );
        detail::log_flag("intrin.h (MSVC)",
#ifdef ALEPH_HAS_INTRIN_H
                         true
#else
                         false
#endif
        );
        detail::log_flag("BMI2 / PEXT",
#ifdef ALEPH_HAS_BMI2
                         true
#else
                         false
#endif
        );
    }

    /**
     * Logs all platform diagnostic information.
     * Must be called after `loggingInit()` during engine startup.
     */
    inline auto logPlatformConfig() -> void { logIntrinsicsConfig(); }

}  // namespace aleph::platform