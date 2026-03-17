/**
 * @file include/aleph/platform/log_info.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <spdlog/spdlog.h>

#include "compiler.hpp"
#include "intrinsics.hpp"

namespace aleph::platform {

    namespace detail {

        /** Column width for flag name alignment in diagnostic output. */
        constexpr int LOG_WIDTH = 36;

        inline auto log_flag(const char* name, bool value) -> void {
            spdlog::info("{:<{}} | {}", name, LOG_WIDTH, value ? "Yes" : "No");
        }

        inline auto log_section(const char* title) -> void {
            spdlog::info("");
            spdlog::info("{}", title);
            spdlog::info("{}", std::string(LOG_WIDTH, '-'));
        }

    } // namespace detail

    /**
     * Logs compiler and language feature configuration.
     * Must be called after `loggingInit()`.
     */
    inline auto logCompilerConfig() -> void {
        detail::log_section("Language / Feature Support");

        detail::log_flag("[[assume]]",
#ifdef ALEPH_HAS_ASSUME
            true
#else
            false
#endif
        );
    }

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
    inline auto logPlatformConfig() -> void {
        logCompilerConfig();
        logIntrinsicsConfig();
    }

} // namespace aleph::platform