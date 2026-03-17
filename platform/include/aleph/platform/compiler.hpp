/**
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <cassert>

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
    #define ALEPH_ASSUME(x, msg) [[assume(x)]] assert(x, msg)
    /**
     * When defined, the `ALEPH_ASSUME` macro will emit the C++23 `[[assume]]`` attribute.
     */
    #define ALEPH_HAS_ASSUME
#else
    #define ALEPH_ASSUME(x, msg) assert(x, msg)
#endif
