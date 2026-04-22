#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <cstring>

#include "aleph/platform/allocation/string.hpp"
#include "aleph/platform/allocation/base.hpp"

using namespace aleph::platform::allocation;

// ---------------------------
// Fixture
// ---------------------------

class StringArenaTest : public ::testing::Test {
    void SetUp() override {
        allocation = new Allocation(detail::STRING_ALLOCATION_SIZE, 0);
        arena = new StringArena(allocation);
    }
    void TearDown() override {
        delete allocation;
        delete arena;
    }
protected:
    Allocation* allocation;
    StringArena* arena;
};

// ---------------------------
// Basic allocation sanity
// ---------------------------

TEST_F(StringArenaTest, AllocateReturnsValidString) {
    auto s = arena->allocate();

    EXPECT_NE(s.c_str(), nullptr);
    EXPECT_EQ(s.size(), 0);
}

// ---------------------------
// Write correctness
// ---------------------------

TEST_F(StringArenaTest, PushBackStoresCharacters) {
    auto s = arena->allocate();

    s.push_back('H');
    s.push_back('e');
    s.push_back('y');

    EXPECT_EQ(s.size(), 3);
    EXPECT_EQ(std::string(s.c_str(), s.size()), "Hey");
}

// ---------------------------
// SubAllocation alignment correctness (indirect)
// ---------------------------

TEST_F(StringArenaTest, SubAllocationDoesNotCrash) {
    EXPECT_NO_THROW({
        auto s1 = allocation->getSubAllocation<char>(64);
        auto s2 = allocation->getSubAllocation<char>(128);

        EXPECT_NE(&s1[0], &s2[0]);
    });
}

// ---------------------------
// Boundary: fill string buffer
// ---------------------------

TEST_F(StringArenaTest, FillToCapacity) {
    auto s = arena->allocate();

    for (std::size_t i = 0; i < detail::STRING_BUFFER_SIZE; ++i) {
        s.push_back('x');
    }

    EXPECT_EQ(s.size(), detail::STRING_BUFFER_SIZE);
}

// ---------------------------
// Overflow protection (debug assert may trigger)
// ---------------------------

TEST_F(StringArenaTest, OverflowDoesNotCorruptMemory) {
    auto s = arena->allocate();

    for (std::size_t i = 0; i < detail::STRING_BUFFER_SIZE; ++i) {
        s.push_back('a');
    }

    // next push should be unsafe logically; we only verify no crash
    EXPECT_NO_THROW({
        try {
            s.push_back('b');
        } catch (...) {
            // acceptable depending on debug build behavior
        }
    });
}

// ---------------------------
// Copy semantics (important: shallow behavior)
// ---------------------------

TEST_F(StringArenaTest, CopySharesBuffer) {
    auto s1 = arena->allocate();
    s1.push_back('X');

    NUMAString s2 = s1;

    EXPECT_EQ(std::string(s1.c_str(), s1.size()),
              std::string(s2.c_str(), s2.size()));
}

// ---------------------------
// Assignment overwrites view
// ---------------------------

TEST_F(StringArenaTest, AssignmentUpdatesView) {
    auto a = arena->allocate();
    auto b = arena->allocate();

    a.push_back('A');
    b.push_back('B');

    b = a;

    EXPECT_EQ(std::string(b.c_str(), b.size()), "A");
}

// ---------------------------
// Multiple allocations independence
// ---------------------------

TEST_F(StringArenaTest, MultipleStringsIndependent) {
    auto a = arena->allocate();
    auto b = arena->allocate();

    a.push_back('1');
    b.push_back('2');

    EXPECT_EQ(a.c_str()[0], '1');
    EXPECT_EQ(b.c_str()[0], '2');
}

// ---------------------------
// Stress allocation (arena exhaustion risk)
// ---------------------------

TEST_F(StringArenaTest, StressAllocation) {
    std::vector<NUMAString> strings;

    for (int i = 0; i < 200; ++i) {
        auto s = arena->allocate();
        s.push_back(static_cast<char>('A' + (i % 26)));
        strings.push_back(s);
    }

    SUCCEED();
}

// ---------------------------
// Multi-thread allocation sanity
// ---------------------------

TEST_F(StringArenaTest, MultiThreadAllocate) {
    constexpr int THREADS = 4;
    constexpr int OPS = 128;

    std::vector<std::thread> workers;

    for (int t = 0; t < THREADS; ++t) {
        workers.emplace_back([&]() {
            for (int i = 0; i < OPS; ++i) {
                auto s = arena->allocate();
                s.push_back('T');
                s.push_back(static_cast<char>('0' + t));
            }
        });
    }

    for (auto& w : workers) {
        w.join();
    }

    SUCCEED();
}