/**
 * @file tests/platform/intrinsics.cpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

#include <aleph/platform.hpp>

using namespace aleph::platform;

namespace {
    constexpr std::uint64_t MAX_U64 = std::numeric_limits<std::uint64_t>::max();

    TEST(PEXT, ZeroInputs) {
        EXPECT_EQ(detail::pext(0, 0), 0);
        EXPECT_EQ(detail::pext(MAX_U64, 0), 0);
        EXPECT_EQ(detail::pext(0, MAX_U64), 0);

        EXPECT_EQ(pext(0, 0), 0);
        EXPECT_EQ(pext(MAX_U64, 0), 0);
        EXPECT_EQ(pext(0, MAX_U64), 0);
    }

    TEST(PEXT, KnownValues) {
        EXPECT_EQ(detail::pext(0b110101, 0b101010), 0b100);
        EXPECT_EQ(detail::pext(0b110101, 0b111111), 0b110101);
        EXPECT_EQ(detail::pext(0b101100, 0b001101), 0b110);
        EXPECT_EQ(detail::pext(MAX_U64, MAX_U64), MAX_U64);

        EXPECT_EQ(pext(0b110101, 0b101010), 0b100);
        EXPECT_EQ(pext(0b110101, 0b111111), 0b110101);
        EXPECT_EQ(pext(0b101100, 0b001101), 0b110);
        EXPECT_EQ(pext(MAX_U64, MAX_U64), MAX_U64);
    }

    TEST(PEXT, SingleBitMasks) {
        EXPECT_EQ(pext(0x0000000000000001ULL, 0x0000000000000001ULL), 1);
        EXPECT_EQ(pext(0x8000000000000000ULL, 0x8000000000000000ULL), 1);
        EXPECT_EQ(pext(0x8000000000000000ULL, 0x0000000000000001ULL), 0);
        EXPECT_EQ(pext(0x0000000000000001ULL, 0x8000000000000000ULL), 0);

        EXPECT_EQ(pext(0xAAAAAAAAAAAAAAAAULL, 0xAAAAAAAAAAAAAAAAULL), 0xFFFFFFFFULL);
        EXPECT_EQ(pext(0x5555555555555555ULL, 0x5555555555555555ULL), 0xFFFFFFFFULL);
    }

    TEST(PEXT, SparseAndDenseMasks) {
        EXPECT_EQ(pext(0xFFFFFFFFFFFFFFFFULL, 0xAAAAAAAAAAAAAAAAULL), 0xFFFFFFFFULL);
        EXPECT_EQ(pext(0xFFFFFFFFFFFFFFFFULL, 0x5555555555555555ULL), 0xFFFFFFFFULL);
        EXPECT_EQ(pext(0xAAAAAAAAAAAAAAAAULL, 0x5555555555555555ULL), 0);
        EXPECT_EQ(pext(0x5555555555555555ULL, 0xAAAAAAAAAAAAAAAAULL), 0);

        EXPECT_EQ(pext(0x123456789ABCDEF0ULL, 0x000000000000FFFFULL), 0xDEF0ULL);
        EXPECT_EQ(pext(0x123456789ABCDEF0ULL, 0xFFFF000000000000ULL), 0x1234ULL);
    }

    TEST(HiMul64, ZeroAndIdentity) {
        EXPECT_EQ(detail::hi_mul64(0, 0), 0);
        EXPECT_EQ(detail::hi_mul64(MAX_U64, 0), 0);
        EXPECT_EQ(detail::hi_mul64(0, MAX_U64), 0);
        EXPECT_EQ(detail::hi_mul64(1, MAX_U64), 0);

        EXPECT_EQ(hi_mul64(0, 0), 0);
        EXPECT_EQ(hi_mul64(MAX_U64, 0), 0);
        EXPECT_EQ(hi_mul64(0, MAX_U64), 0);
        EXPECT_EQ(hi_mul64(1, MAX_U64), 0);
    }

    TEST(HiMul64, KnownValues) {
        EXPECT_EQ(detail::hi_mul64(MAX_U64, MAX_U64), 0xFFFFFFFFFFFFFFFEULL);
        EXPECT_EQ(detail::hi_mul64(0xFFFFFFFFULL, 0xFFFFFFFFULL), 0);
        EXPECT_EQ(detail::hi_mul64(0x100000000ULL, 0x100000000ULL), 1);
        EXPECT_EQ(detail::hi_mul64(0x8000000000000000ULL, 2), 1);
        EXPECT_EQ(detail::hi_mul64(0xFFFFFFFF00000000ULL, 0xFFFFFFFF00000000ULL),
                  0xFFFFFFFE00000001ULL);

        EXPECT_EQ(hi_mul64(MAX_U64, MAX_U64), 0xFFFFFFFFFFFFFFFEULL);
        EXPECT_EQ(hi_mul64(0xFFFFFFFFULL, 0xFFFFFFFFULL), 0);
        EXPECT_EQ(hi_mul64(0x100000000ULL, 0x100000000ULL), 1);
        EXPECT_EQ(hi_mul64(0x8000000000000000ULL, 2), 1);
        EXPECT_EQ(hi_mul64(0xFFFFFFFF00000000ULL, 0xFFFFFFFF00000000ULL), 0xFFFFFFFE00000001ULL);
    }

    TEST(HiMul64, BoundaryValues) {
        EXPECT_EQ(hi_mul64(0xFFFFFFFFFFFFFFFFULL, 0x0000000100000000ULL), 0xFFFFFFFFULL);
        EXPECT_EQ(hi_mul64(0xFFFFFFFF00000000ULL, 0x00000000FFFFFFFFULL), 0xFFFFFFFEULL);
        EXPECT_EQ(hi_mul64(0x0000000100000000ULL, 0x00000000FFFFFFFFULL), 0);
        EXPECT_EQ(hi_mul64(0x0000000200000000ULL, 0x0000000080000000ULL), 1);
        EXPECT_EQ(hi_mul64(0x0000000180000000ULL, 0x0000000180000000ULL), 2);
    }
}  // namespace