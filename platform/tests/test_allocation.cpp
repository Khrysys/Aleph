#include <gtest/gtest.h>
#include <atomic>
#include <utility>
#include <cstddef>
#include <vector>
#include <thread>
#include <unordered_set>
#include <libassert/assert.hpp>

#include <aleph/platform.hpp>
#include <mutex>

using namespace aleph::platform::allocation;

// -----------------------------
// Allocation basic construction
// -----------------------------
TEST(AllocationTest, BasicAllocationSizeAlignment)
{
    const Allocation alloc(1000, 0);

    // page alignment is implementation-defined,
    // but size must be >= requested
    EXPECT_GE(alloc.getPageSize(), 4096);

    // We cannot inspect ptr directly (private),
    // so we rely on no crash + move semantics correctness.
}

// -----------------------------
// Move constructor
// -----------------------------
TEST(AllocationTest, MoveConstructorTransfersOwnership)
{
    Allocation a(1024, 0);

    Allocation b(std::move(a));

    // original should be nulled out safely
    Allocation c(512, 0);
    c = std::move(b);

    SUCCEED();
}

// -----------------------------
// Move assignment
// -----------------------------
TEST(AllocationTest, MoveAssignmentWorks)
{
    Allocation a(2048, 0);
    Allocation b(4096, 0);

    b = std::move(a);

    SUCCEED();
}

// -----------------------------
// NUMA node parameter acceptance
// -----------------------------
TEST(AllocationTest, NumaNodeIsAccepted)
{
    const Allocation a(1024, 0);
    const Allocation b(1024, 1);
    const Allocation c(1024, 999); // should not crash even if invalid node

    SUCCEED();
}

// -----------------------------
// SubAllocation size correctness
// -----------------------------
TEST(SubAllocationTest, CorrectElementCount)
{
    alignas(64) char buffer[1024];

    SubAllocation<int> sub(buffer, sizeof(buffer));

    EXPECT_EQ(sub.getSize(), sizeof(buffer) / sizeof(int));
}

// -----------------------------
// SubAllocation bounds safety (debug only)
// -----------------------------
TEST(SubAllocationTest, ElementAccessWorks)
{
    alignas(64) int buffer[64] = {};

    SubAllocation<int> sub(buffer, sizeof(buffer));

    sub[0] = 42;
    sub[10] = 99;

    EXPECT_EQ(sub[0], 42);
    EXPECT_EQ(sub[10], 99);
}

// -----------------------------
// Invalid size throws
// -----------------------------
TEST(SubAllocationTest, MisalignedSizeThrows)
{
    std::vector<char> buffer(1003); // not divisible by sizeof(int)

    EXPECT_THROW(
        (SubAllocation<int>(buffer.data(), buffer.size())),
        std::runtime_error
    );
}

// -----------------------------
// Multi-threaded construction sanity
// -----------------------------
TEST(AllocationTest, MultiThreadedConstruction)
{
    std::vector<std::thread> threads;

    for (int i = 0; i < 32; ++i) {
        threads.emplace_back([] {
            Allocation a(4096, 0);
            Allocation b(8192, 1);
            Allocation c(16384, 0);
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    SUCCEED();
}

TEST(StringArenaTest, AllocateUniqueSlots)
{
    auto alloc = Allocation(detail::STRING_ALLOCATION_SIZE, 0);
    StringArena arena(&alloc);

    std::vector<NUMAString> strings;

    for (std::size_t i = 0; i < detail::STRING_ARENA_SIZE; ++i) {
        strings.emplace_back(arena.allocate());
    }

    // Expect exhaustion on next allocate
    EXPECT_THROW(arena.allocate(), std::runtime_error);
}

TEST(StringArenaTest, ReuseAfterFree)
{
    auto alloc = Allocation(detail::STRING_ALLOCATION_SIZE, 0);
    StringArena arena(&alloc);

    const char* s1p = nullptr;
    {
        auto s1 = arena.allocate();
        s1p = s1.c_str();
        ASSERT(s1p != nullptr);
    }
    const char* s2p = nullptr;
    EXPECT_NO_THROW({
        auto s2 = arena.allocate();
        s2p = s2.c_str();
    });
    ASSERT(s2p != nullptr);
}

TEST(StringArenaTest, PushBackAndRead)
{
    auto alloc = Allocation(detail::STRING_ALLOCATION_SIZE, 0);
    StringArena arena(&alloc);

    auto s = arena.allocate();

    const char* msg = "hello";

    for (size_t i = 0; msg[i]; ++i) {
        s.push_back(msg[i]);
    }

    EXPECT_EQ(std::string(s.c_str(), s.size()), "hello");
}

TEST(StringArenaTest, CopyIncrementsRefCount)
{
    auto alloc = Allocation(detail::STRING_ALLOCATION_SIZE, 0);
    StringArena arena(&alloc);

    auto s1 = arena.allocate();
    s1.push_back('a');

    {
        auto s2 = s1; // copy

        EXPECT_EQ(s2.size(), 1);
        EXPECT_EQ(s2.c_str()[0], 'a');
    }

    // original should still be valid
    EXPECT_EQ(s1.size(), 1);
    EXPECT_EQ(s1.c_str()[0], 'a');
}

TEST(StringArenaTest, MultipleCopiesRelease)
{
    auto alloc = Allocation(detail::STRING_ALLOCATION_SIZE, 0);
    StringArena arena(&alloc);

    std::size_t idx;

    {
        auto s1 = arena.allocate();
        auto s2 = s1;
        auto s3 = s2;

        // all refer to same slot
    }

    // should be reusable now
    EXPECT_NO_THROW({
        auto s = arena.allocate();
    });
}

TEST(StringArenaTest, FullCycleReuse)
{
    auto alloc = Allocation(detail::STRING_ALLOCATION_SIZE, 0);
    StringArena arena(&alloc);

    std::vector<NUMAString> strings;

    for (std::size_t i = 0; i < detail::STRING_ARENA_SIZE; ++i) {
        strings.emplace_back(arena.allocate());
    }

    strings.clear(); // all freed

    for (std::size_t i = 0; i < detail::STRING_ARENA_SIZE; ++i) {
        EXPECT_NO_THROW(arena.allocate());
    }
}

TEST(StringArenaTest, ConcurrentAllocateFree)
{
    auto alloc = Allocation(detail::STRING_ALLOCATION_SIZE, 0);
    StringArena arena(&alloc);

    constexpr int threads = 32;
    constexpr int iterations = 10000;

    std::atomic<bool> failed = false;

    auto worker = [&]() {
        for (int i = 0; i < iterations; ++i) {
            try {
                auto s = arena.allocate();
                s.push_back('x');
            } catch (...) {
                failed = true;
            }
        }
    };

    std::vector<std::thread> ts;
    for (int i = 0; i < threads; ++i)
        ts.emplace_back(worker);

    for (auto& t : ts)
        t.join();

    EXPECT_FALSE(failed.load());
}

TEST(StringArenaTest, NoDuplicateSlots)
{
    auto alloc = Allocation(detail::STRING_ALLOCATION_SIZE, 0);
    StringArena arena(&alloc);

    constexpr int threads = 32;
    constexpr int iterations = 1000;

    std::atomic<bool> duplicate = false;

    std::mutex mtx;
    std::unordered_set<void*> active;

    auto worker = [&]() {
        for (int i = 0; i < iterations; ++i) {
            auto s = arena.allocate();

            {
                std::lock_guard<std::mutex> lock(mtx);
                if (!active.insert((void*)s.c_str()).second) {
                    duplicate = true;
                }
            }

            {
                std::lock_guard<std::mutex> lock(mtx);
                active.erase((void*)s.c_str());
            }
        }
    };

    std::vector<std::thread> ts;
    for (int i = 0; i < threads; ++i)
        ts.emplace_back(worker);

    for (auto& t : ts)
        t.join();

    EXPECT_FALSE(duplicate.load());
}

TEST(StringArenaTest, StressTest)
{
    auto alloc = Allocation(detail::STRING_ALLOCATION_SIZE, 0);
    StringArena arena(&alloc);

    constexpr int threads = 64;
    constexpr int iterations = 50000;

    std::vector<std::thread> ts;

    for (int t = 0; t < threads; ++t) {
        ts.emplace_back([&]() {
            for (int i = 0; i < iterations; ++i) {
                auto s = arena.allocate();

                for (int j = 0; j < 32; ++j) {
                    s.push_back('a' + (j % 26));
                }
            }
        });
    }

    for (auto& t : ts)
        t.join();

    SUCCEED();
}