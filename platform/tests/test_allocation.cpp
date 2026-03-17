/**
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <gtest/gtest.h>

#include <aleph/platform.hpp>

using namespace aleph::platform::allocation;
using namespace aleph::platform;

// ===== allocation_init tests =====
TEST(AllocationInit, IsHugePagesAvailableReturnsBool) {
    // Should not crash and return a valid bool regardless of system support
    EXPECT_NO_FATAL_FAILURE(auto r = isHugePagesAvailable());
}

TEST(AllocationInit, RequestHugePagesReturnsBool) {
    // Should not crash and return a valid bool regardless of system support
    EXPECT_NO_FATAL_FAILURE(auto r = requestHugePages());
}

TEST(AllocationInit, GetPageSizeNonZero) { EXPECT_GT(getPageSize(), std::size_t{0}); }

TEST(AllocationInit, GetPageSizePowerOfTwo) {
    std::size_t page = getPageSize();
    EXPECT_EQ(page & (page - 1), std::size_t{0});
}

TEST(AllocationInit, GetPageSizeCached) {
    // Must return identical value on repeated calls
    EXPECT_EQ(getPageSize(), getPageSize());
}

// ===== roundToPage tests =====

TEST(RoundToPage, AlreadyAligned) {
    std::size_t page = getPageSize();
    EXPECT_EQ(roundToPage(page, page), page);
}

TEST(RoundToPage, ZeroRoundsToZero) {
    std::size_t page = getPageSize();
    EXPECT_EQ(roundToPage(0, page), std::size_t{0});
}

TEST(RoundToPage, OneRoundsUpToPageSize) {
    std::size_t page = getPageSize();
    EXPECT_EQ(roundToPage(1, page), page);
}

TEST(RoundToPage, PageSizeMinusOneRoundsUp) {
    std::size_t page = getPageSize();
    EXPECT_EQ(roundToPage(page - 1, page), page);
}

TEST(RoundToPage, PageSizePlusOneRoundsUpToTwo) {
    std::size_t page = getPageSize();
    EXPECT_EQ(roundToPage(page + 1, page), page * 2);
}

TEST(RoundToPage, MultipleOfPageSizeUnchanged) {
    std::size_t page = getPageSize();
    EXPECT_EQ(roundToPage(page * 4, page), page * 4);
}

TEST(RoundToPage, LargeSize) {
    std::size_t page  = getPageSize();
    std::size_t large = page * 1024;
    EXPECT_EQ(roundToPage(large, page), large);
}

TEST(RoundToPage, ResultAlwaysMultipleOfPageSize) {
    std::size_t page = getPageSize();
    for (size_t i = 0; i <= page * 2; i++) {
        EXPECT_EQ(roundToPage(i, page) % page, std::size_t{0});
    }
}

// ===== allocation_runtime tests =====

TEST(AllocationRuntime, AllocateStandardPage) {
    std::size_t page        = getPageSize();
    AllocationResult result = allocate(page);

    EXPECT_NE(result.ptr, nullptr);
    EXPECT_EQ(result.size, page);
    EXPECT_TRUE(result.page_size == PageSize::Standard || result.page_size == PageSize::Large);

    deallocate(result);
}

TEST(AllocationRuntime, AllocateMultiplePages) {
    std::size_t page        = getPageSize();
    std::size_t size        = page * 4;
    AllocationResult result = allocate(size);

    EXPECT_NE(result.ptr, nullptr);
    EXPECT_EQ(result.size, size);

    deallocate(result);
}

TEST(AllocationRuntime, AllocatedMemoryIsReadWrite) {
    std::size_t page        = getPageSize();
    AllocationResult result = allocate(page);
    ASSERT_NE(result.ptr, nullptr);

    // Write and read back
    std::uint8_t* mem = static_cast<uint8_t*>(result.ptr);
    for (size_t i = 0; i < page; ++i) {
        mem[i] = static_cast<uint8_t>(i & 0xFF);
    }
    for (size_t i = 0; i < page; ++i) {
        EXPECT_EQ(mem[i], static_cast<uint8_t>(i & 0xFF));
    }

    deallocate(result);
}

TEST(AllocationRuntime, DeallocateNullptrIsNoop) {
    AllocationResult result = {nullptr, 0, PageSize::Standard};
    EXPECT_NO_FATAL_FAILURE(deallocate(result));
}

TEST(AllocationRuntime, AllocateRoundedSize) {
    std::size_t page        = getPageSize();
    std::size_t raw         = page * 3 + 1;
    std::size_t size        = roundToPage(raw, page);

    AllocationResult result = allocate(size);
    EXPECT_NE(result.ptr, nullptr);
    EXPECT_EQ(result.size, size);

    deallocate(result);
}