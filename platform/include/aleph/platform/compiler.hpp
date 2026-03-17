/**
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <stdexcept>

#include <absl/log/log.h>
#include <boost/predef.h>

#include "logging.hpp"

#ifndef ALEPH_DEBUG
    #ifndef NDEBUG
        /**
         * When defined, debug logging and exceptions in hot paths are active.
         *
         * Currently, LibTorch in C++20 in modes other than Release fail build.
         * Therefore, as a temporary workaround, since I believe it to be more
         * a build script setup problem rather than a code problem, we'll use
         * this macro and the below `ALEPH_ASSERT` for things that must always
         * assert, and `ALEPH_DEBUG_ASSERT` for when things should not assert
         * in the final build, but they should during development.
         *
         * `ALEPH_DEBUG_ASSERT` is controlled by the `ALEPH_DEBUG` macro. When
         * `ALEPH_DEBUG` is not defined, `ALEPH_DEBUG_ASSERT` does nothing.
         * When `ALEPH_DEBUG` is defined, `ALEPH_DEBUG_ASSERT` is equivalent to
         * `ALEPH_ASSERT`.
         */
        #define ALEPH_DEBUG
    #endif
#endif

/**
 * Asserts a condition in all build configurations.
 *
 * Logs the failure via absl LOG(ERROR) and throws std::logic_error.
 * Requires logging to be initialized via `aleph::platform::loggingInit()`
 * before use; behavior is undefined if called before logging is ready.
 */
#define ALEPH_ASSERT(condition, msg)                                                     \
    do {                                                                                 \
        if (!(condition)) {                                                              \
            ALEPH_LOG(ERROR) << "Assertion failed: " << (msg) << " [" << __FILE__ << ":" \
                             << __LINE__ << "]";                                         \
            throw std::logic_error(msg);                                                 \
        }                                                                                \
    } while (0)

/**
 * Asserts a condition only when `ALEPH_DEBUG` is defined.
 *
 * Equivalent to `ALEPH_ASSERT` when `ALEPH_DEBUG` is defined, and a no-op
 * otherwise. Use for hot-path precondition checks that should not be enforced
 * in production builds.
 *
 * Requires logging to be initialized via `aleph::platform::loggingInit()`
 * before use; behavior is undefined if called before logging is ready.
 *
 * @param x   The condition to assert.
 * @param msg Description of the assumption, used in the debug assertion.
 */
#ifdef ALEPH_DEBUG
    #define ALEPH_DEBUG_ASSERT(x, msg) ALEPH_ASSERT(condition, msg)
#else
    #define ALEPH_DEBUG_ASSERT(x, msg) ((void)0)
#endif

/**
 * Hints to the compiler that `x` can be assumed true for optimization purposes.
 *
 * On compilers supporting C++23 `[[assume]]`, emits the attribute. In debug
 * builds, also enforces the condition via `ALEPH_DEBUG_ASSERT`. In release
 * builds without `[[assume]]` support, degrades to a no-op.
 *
 * @param x   The condition to assume.
 * @param msg Description of the assumption, used in the debug assertion.
 */
#if __has_cpp_attribute(assume) >= 202207L
    #define ALEPH_ASSUME(x, msg) [[assume(x)]] ALEPH_DEBUG_ASSERT(x, msg)
    /**
     * When defined, the `ALEPH_ASSUME` macro will emit the C++23 `[[assume]]`` attribute.
     */
    #define ALEPH_HAS_ASSUME
#else
    #define ALEPH_ASSUME(x, msg) ALEPH_DEBUG_ASSERT(x, msg)
#endif
