#pragma once

#include <cstdint>

#include "compiler.hpp"

namespace aleph::platform {
    [[nodiscard]] constexpr auto splitMix64(std::uint64_t& key) {
        std::uint64_t val = (key += 0x9e3779b97f4a7c15);
        val               = (val ^ (val >> 30)) * 0xbf58476d1ce4e5b9;
        val               = (val ^ (val >> 27)) * 0x94d049bb133111eb;
        return val ^ (val >> 31);
    }

    [[nodiscard]] constexpr auto hi_mul64(std::uint64_t lhs, std::uint64_t rhs) {
        std::uint64_t highResult;
#if BOOST_OS_WINDOWS
        if (std::is_constant_evaluated()) {
            std::uint64_t lhs_hi = lhs >> 32;
            std::uint64_t lhs_lo = lhs & 0xFFFFFFFF;
            std::uint64_t rhs_hi = rhs >> 32;
            std::uint64_t rhs_lo = rhs & 0xFFFFFFFF;

            std::uint64_t p00 = lhs_lo * rhs_lo;
            std::uint64_t p01 = lhs_lo * rhs_hi;
            std::uint64_t p10 = lhs_hi * rhs_lo;
            std::uint64_t p11 = lhs_hi * rhs_hi;

            std::uint64_t middle = p10 + (p00 >> 32) + (p01 & 0xFFFFFFFF);  
            highResult = p11 + (middle >> 32) + (p01 >> 32);
        } else {
            _umul128(lhs, rhs, &highResult);
        }
#else
        __uint128_t total = (__uint128_t)lhs * (__uint128_t)rhs;
        highResult        = total >> 64;
#endif
        return highResult;
    }
}  // namespace aleph::platform