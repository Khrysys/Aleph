/**
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <bit>
#include <cstdint>
#include <type_traits>

#include "compiler.hpp"

// ===== Intrinsics Includes =====
#if __has_include(<x86intrin.h>)
    #include <x86intrin.h>
    #define ALEPH_HAS_X86INTRIN_H
#endif
#if __has_include(<intrin.h>)
    #include <intrin.h>
    #define ALEPH_HAS_INTRIN_H
#endif

/**
 * Defined when BMI2 is available on the target platform.
 * Enables hardware-accelerated `pext` via `_pext_u64`.
 */
#ifdef __BMI2__
    #define ALEPH_HAS_BMI2
#endif

namespace aleph::platform {

    namespace detail {
        /**
         * Software fallback implementation of PEXT (parallel bits extract).
         *
         * Extracts bits from `src` at positions indicated by set bits in `mask`,
         * packing them contiguously into the result starting from bit 0.
         * Used at compile time and as a fallback when BMI2 is unavailable.
         *
         * @param src  Source value to extract bits from.
         * @param mask Bitmask indicating which bits to extract.
         * @return Extracted bits packed into the low bits of the result.
         */
        [[nodiscard]] inline constexpr uint64_t pext(uint64_t src, uint64_t mask) noexcept {
            uint64_t result = 0;
            uint64_t bit    = 1;
            while (mask) {
                uint64_t lowest = mask & -mask;
                if (src & lowest) result |= bit;
                mask  &= mask - 1;
                bit  <<= 1;
            }
            return result;
        }
    }  // namespace detail

    /**
     * Returns the number of set bits in `x`.
     * Delegates to `std::popcount` which emits hardware POPCNT where available.
     *
     * @param x Value to count set bits in.
     * @return Number of set bits.
     */
    [[nodiscard]] inline constexpr uint64_t popcnt(uint64_t x) noexcept { return std::popcount(x); }

    /**
     * Returns the number of leading zero bits in `x`.
     * Delegates to `std::countl_zero` which emits hardware LZCNT where available.
     *
     * @param x Value to count leading zeros in.
     * @return Number of leading zero bits. Returns 64 if `x` is zero.
     */
    [[nodiscard]] inline constexpr uint64_t lzcnt(uint64_t x) noexcept {
        return std::countl_zero(x);
    }

    /**
     * Returns the number of trailing zero bits in `x`.
     * Delegates to `std::countr_zero` which emits hardware TZCNT where available.
     *
     * @param x Value to count trailing zeros in.
     * @return Number of trailing zero bits. Returns 64 if `x` is zero.
     */
    [[nodiscard]] inline constexpr uint64_t tzcnt(uint64_t x) noexcept {
        return std::countr_zero(x);
    }

    /**
     * Extracts bits from `src` at positions indicated by set bits in `mask`,
     * packing them contiguously into the result starting from bit 0.
     *
     * At compile time, uses the software fallback in `detail::pext`.
     * At runtime, uses `_pext_u64` when BMI2 is available, otherwise
     * falls back to the software implementation.
     *
     * No STL equivalent exists as of C++23.
     *
     * @param src  Source value to extract bits from.
     * @param mask Bitmask indicating which bits to extract.
     * @return Extracted bits packed into the low bits of the result.
     */
    [[nodiscard]] inline constexpr uint64_t pext(uint64_t src, uint64_t mask) noexcept {
        if (std::is_constant_evaluated()) {
            return detail::pext(src, mask);
        }
#if defined(ALEPH_HAS_BMI2) && (defined(ALEPH_HAS_X86INTRIN_H) || defined(ALEPH_HAS_INTRIN_H))
        return _pext_u64(src, mask);
#else
        return detail::pext(src, mask);
#endif
    }

}  // namespace aleph::platform