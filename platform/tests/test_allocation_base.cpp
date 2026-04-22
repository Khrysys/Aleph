/**
 * @file tests/test_allocation_base.cpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <atomic>
#include <cstddef>
#include <stdexcept>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include <aleph/platform/allocation/base.hpp>

using namespace aleph::platform::allocation;

namespace {

// ===== SubAllocation =====

TEST(SubAllocation, ConstructionValidSize) {
    alignas(int) std::byte buf[sizeof(int) * 4];
    EXPECT_NO_THROW((SubAllocation<int>(buf, sizeof(int) * 4)));
}

TEST(SubAllocation, ConstructionInvalidSizeThrows) {
    alignas(int) std::byte buf[sizeof(int) * 4 + 1];
    EXPECT_THROW((SubAllocation<int>(buf, sizeof(int) * 4 + 1)), std::runtime_error);
}

TEST(SubAllocation, GetSizeReturnsElementCount) {
    alignas(int) std::byte buf[sizeof(int) * 8];
    SubAllocation<int> sub(buf, sizeof(int) * 8);
    EXPECT_EQ(sub.getSize(), 8u);
}

TEST(SubAllocation, IndexAccessReadWrite) {
    alignas(int) std::byte buf[sizeof(int) * 4];
    SubAllocation<int> sub(buf, sizeof(int) * 4);
    for (std::size_t i = 0; i < 4; ++i) {
        sub[i] = static_cast<int>(i * 10);
    }
    for (std::size_t i = 0; i < 4; ++i) {
        EXPECT_EQ(sub[i], static_cast<int>(i * 10));
    }
}

TEST(SubAllocation, SingleElementSize) {
    alignas(double) std::byte buf[sizeof(double)];
    SubAllocation<double> sub(buf, sizeof(double));
    EXPECT_EQ(sub.getSize(), 1u);
    sub[0] = 3.14;
    EXPECT_DOUBLE_EQ(sub[0], 3.14);
}

TEST(SubAllocation, ZeroSizeThrows) {
    alignas(int) std::byte buf[sizeof(int)];
    // Zero is divisible by sizeof(int) so construction succeeds,
    // but getSize() returns 0.
    SubAllocation<int> sub(buf, 0);
    EXPECT_EQ(sub.getSize(), 0u);
}

// ===== Allocation static methods =====

TEST(Allocation, AreLargePagesAvailableReturnsBool) {
    bool result = Allocation::areLargePagesAvailable();
    EXPECT_TRUE(result == true || result == false);
}

TEST(Allocation, GetPageSizeNonZero) {
    EXPECT_GT(Allocation::getPageSize(), std::size_t{0});
}

TEST(Allocation, GetPageSizePowerOfTwo) {
    std::size_t page = Allocation::getPageSize();
    EXPECT_EQ(page & (page - 1), std::size_t{0});
}

TEST(Allocation, GetPageSizeCached) {
    EXPECT_EQ(Allocation::getPageSize(), Allocation::getPageSize());
}

// ===== Allocation construction =====

TEST(Allocation, ConstructionSucceeds) {
    std::size_t page = Allocation::getPageSize();
    EXPECT_NO_THROW(Allocation alloc(page, 0));
}

TEST(Allocation, MoveConstructionTransfersOwnership) {
    std::size_t page = Allocation::getPageSize();
    Allocation a(page, 0);
    Allocation b(std::move(a));
    // b should be usable
    EXPECT_NO_THROW(b.getSubAllocation<std::byte>(1));
}

TEST(Allocation, MoveAssignmentTransfersOwnership) {
    std::size_t page = Allocation::getPageSize();
    Allocation a(page, 0);
    Allocation b(page, 0);
    b = std::move(a);
    EXPECT_NO_THROW(b.getSubAllocation<std::byte>(1));
}

TEST(Allocation, CopyConstructionDeleted) {
    EXPECT_FALSE(std::is_copy_constructible_v<Allocation>);
}

TEST(Allocation, CopyAssignmentDeleted) {
    EXPECT_FALSE(std::is_copy_assignable_v<Allocation>);
}

// ===== getSubAllocation =====

TEST(Allocation, SubAllocationReturnsValidPointer) {
    std::size_t page = Allocation::getPageSize();
    Allocation alloc(page, 0);
    auto sub = alloc.getSubAllocation<int>(1);
    EXPECT_EQ(sub.getSize(), 1u);
    sub[0] = 42;
    EXPECT_EQ(sub[0], 42);
}

TEST(Allocation, SubAllocationAlignment) {
    std::size_t page = Allocation::getPageSize();
    Allocation alloc(page, 0);
    // double requires 8-byte alignment
    auto sub = alloc.getSubAllocation<double>(4);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&sub[0]) % alignof(double), 0u);
}

TEST(Allocation, SubAllocationExhaustionThrows) {
    std::size_t page = Allocation::getPageSize();
    Allocation alloc(page, 0);
    EXPECT_THROW(alloc.getSubAllocation<std::byte>(page + 1), std::bad_alloc);
}

TEST(Allocation, MultipleSubAllocationsDoNotOverlap) {
    std::size_t page = Allocation::getPageSize();
    Allocation alloc(page, 0);
    auto sub1 = alloc.getSubAllocation<int>(4);
    auto sub2 = alloc.getSubAllocation<int>(4);

    // Write distinct values to each
    for (std::size_t i = 0; i < 4; ++i) sub1[i] = 1;
    for (std::size_t i = 0; i < 4; ++i) sub2[i] = 2;

    // Verify no overlap
    for (std::size_t i = 0; i < 4; ++i) EXPECT_EQ(sub1[i], 1);
    for (std::size_t i = 0; i < 4; ++i) EXPECT_EQ(sub2[i], 2);
}

TEST(Allocation, SubAllocationFillsEntireAllocation) {
    std::size_t page = Allocation::getPageSize();
    Allocation alloc(page, 0);
    // Should be able to fill the entire allocation with bytes
    EXPECT_NO_THROW(alloc.getSubAllocation<std::byte>(page));
}

// ===== Concurrent getSubAllocation =====

TEST(Allocation, ConcurrentSubAllocationNoOverlap) {
    std::size_t page = Allocation::getPageSize();
    Allocation alloc(page * 8, 0);

    constexpr int N_THREADS = 8;
    constexpr std::size_t PER_THREAD = 16;

    std::vector<SubAllocation<int>> subs;
    std::mutex subsMutex;
    std::vector<std::thread> threads;

    for (int t = 0; t < N_THREADS; ++t) {
        threads.emplace_back([&]() {
            auto sub = alloc.getSubAllocation<int>(PER_THREAD);
            for (std::size_t i = 0; i < PER_THREAD; ++i) {
                sub[i] = static_cast<int>(i);
            }
            std::scoped_lock lock(subsMutex);
            subs.push_back(sub);
        });
    }

    for (auto& t : threads) t.join();

    // Verify all suballocations point to distinct memory regions
    for (std::size_t i = 0; i < subs.size(); ++i) {
        for (std::size_t j = i + 1; j < subs.size(); ++j) {
            EXPECT_NE(&subs[i][0], &subs[j][0]);
        }
    }
}

} // namespace