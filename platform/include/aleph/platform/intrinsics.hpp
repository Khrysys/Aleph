/**
 * @file include/aleph/platform/intrinsics.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <bit>
#include <cstdint>

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
        [[nodiscard]] constexpr auto pext(
            std::uint64_t src,            // NOLINT(bugprone-easily-swappable-parameters)
            std::uint64_t mask) noexcept  // NOLINT(bugprone-easily-swappable-parameters)
            -> std::uint64_t {
            std::uint64_t result = 0;
            std::uint64_t bit    = 1;
            while (mask != 0ULL) {
                if (const std::uint64_t lowest = mask & (~mask + 1); (src & lowest) != 0ULL) {
                    result |= bit;
                }
                mask  &= mask - 1;
                bit  <<= 1U;
            }
            return result;
        }

        constexpr auto hi_mul64(std::uint64_t lhs, std::uint64_t rhs) {
            std::uint64_t lhs_hi = lhs >> 32;
            std::uint64_t lhs_lo = lhs & 0xFFFFFFFF;
            std::uint64_t rhs_hi = rhs >> 32;
            std::uint64_t rhs_lo = rhs & 0xFFFFFFFF;

            std::uint64_t p00    = lhs_lo * rhs_lo;
            std::uint64_t p01    = lhs_lo * rhs_hi;
            std::uint64_t p10    = lhs_hi * rhs_lo;
            std::uint64_t p11    = lhs_hi * rhs_hi;

            std::uint64_t middle = p10 + (p00 >> 32) + (p01 & 0xFFFFFFFF);
            return p11 + (middle >> 32) + (p01 >> 32);
        }
    }  // namespace detail

    /**
     * Returns the number of set bits in `val`.
     * Delegates to `std::popcount` which emits hardware POPCNT where available.
     *
     * @param val Value to count set bits in.
     * @return Number of set bits.
     */
    [[nodiscard]] constexpr auto popcnt(std::uint64_t val) noexcept { return std::popcount(val); }

    /**
     * Returns the number of leading zero bits in `val`.
     * Delegates to `std::countl_zero` which emits hardware LZCNT where available.
     *
     * @param val Value to count leading zeros in.
     * @return Number of leading zero bits. Returns 64 if `val` is zero.
     */
    [[nodiscard]] constexpr auto lzcnt(std::uint64_t val) noexcept { return std::countl_zero(val); }

    /**
     * Returns the number of trailing zero bits in `val`.
     * Delegates to `std::countr_zero` which emits hardware TZCNT where available.
     *
     * @param val Value to count trailing zeros in.
     * @return Number of trailing zero bits. Returns 64 if `val` is zero.
     */
    [[nodiscard]] constexpr auto tzcnt(std::uint64_t val) noexcept { return std::countr_zero(val); }

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
    [[nodiscard]] constexpr auto pext(std::uint64_t src, std::uint64_t mask) noexcept
        -> std::uint64_t;

    /**
     * Returns the high 64-bits of a 64x64->128-bit multiplication. The distribution of the returned
     * value is uniform within the range `[0, min(lhs, rhs))` assuming uniform distributions for
     * `lhs` and `rhs`.
     */
    [[nodiscard]] constexpr auto hi_mul64(std::uint64_t lhs, std::uint64_t rhs) -> std::uint64_t;

}  // namespace aleph::platform