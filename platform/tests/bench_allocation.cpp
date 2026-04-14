#include <vector>

#include <benchmark/benchmark.h>
#include <aleph/platform.hpp>

using namespace aleph::platform::allocation;

// -----------------------------
// Allocation benchmark
// -----------------------------
static void BM_AllocationConstruction(benchmark::State& state)
{
    for (auto _ : state) {
        Allocation alloc(4096, 0);
        benchmark::DoNotOptimize(alloc);
    }
}
BENCHMARK(BM_AllocationConstruction);

// -----------------------------
// Large allocation benchmark
// -----------------------------
static void BM_LargeAllocation(benchmark::State& state)
{
    for (auto _ : state) {
        Allocation alloc(1 << 20, 0); // 1MB
        benchmark::DoNotOptimize(alloc);
    }
}
BENCHMARK(BM_LargeAllocation);

// -----------------------------
// SubAllocation access benchmark
// -----------------------------
static void BM_SubAllocationAccess(benchmark::State& state)
{
    alignas(64) int buffer[1024];
    SubAllocation<int> sub(buffer, sizeof(buffer));

    for (auto _ : state) {
        for (std::size_t i = 0; i < sub.getSize(); ++i) {
            sub[i] = static_cast<int>(i);
        }
    }
}
BENCHMARK(BM_SubAllocationAccess);

// -----------------------------
// Threaded allocation pressure test
// -----------------------------
static void BM_AllocationMultiThread(benchmark::State& state)
{
    for (auto _ : state) {
        std::vector<std::thread> threads;
        threads.reserve(8);

        for (int i = 0; i < 8; ++i) {
            threads.emplace_back([] {
                for (int j = 0; j < 1000; ++j) {
                    Allocation alloc(4096, 0);
                    benchmark::DoNotOptimize(alloc);
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }
    }
}
BENCHMARK(BM_AllocationMultiThread)->Unit(benchmark::kMillisecond);

static void BM_Allocate_SingleThread(benchmark::State& state)
{
    auto alloc = Allocation(detail::STRING_ALLOCATION_SIZE, 0);
    StringArena arena(&alloc);

    for (auto _ : state) {
        auto s = arena.allocate();
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(BM_Allocate_SingleThread);

static void BM_AllocateAndWrite(benchmark::State& state)
{
    auto alloc = Allocation(detail::STRING_ALLOCATION_SIZE, 0);
    StringArena arena(&alloc);

    for (auto _ : state) {
        auto s = arena.allocate();

        for (int i = 0; i < 32; ++i) {
            s.push_back('a');
        }

        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(BM_AllocateAndWrite);

static void BM_AllocateDestroy(benchmark::State& state)
{
    auto alloc = Allocation(detail::STRING_ALLOCATION_SIZE, 0);
    StringArena arena(&alloc);

    for (auto _ : state) {
        {
            auto s = arena.allocate();
            benchmark::DoNotOptimize(s);
        }
    }
}
BENCHMARK(BM_AllocateDestroy);

static void BM_CopyNUMAString(benchmark::State& state)
{
    auto alloc = Allocation(detail::STRING_ALLOCATION_SIZE, 0);
    StringArena arena(&alloc);
    auto base = arena.allocate();

    for (auto _ : state) {
        auto copy = base;
        benchmark::DoNotOptimize(copy);
    }
}
BENCHMARK(BM_CopyNUMAString);

static void BM_Allocate_MultiThread(benchmark::State& state)
{
    auto alloc = Allocation(detail::STRING_ALLOCATION_SIZE, 0);
    StringArena arena(&alloc);

    for (auto _ : state) {
        auto s = arena.allocate();
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(BM_Allocate_MultiThread)->ThreadRange(1, 64);

static void BM_AllocateFree_MultiThread(benchmark::State& state)
{
    auto alloc = Allocation(detail::STRING_ALLOCATION_SIZE, 0);
    StringArena arena(&alloc);

    for (auto _ : state) {
        auto s = arena.allocate();
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(BM_AllocateFree_MultiThread)->ThreadRange(1, 64);

static void BM_ContentionStress(benchmark::State& state)
{
    auto alloc = Allocation(detail::STRING_ALLOCATION_SIZE, 0);
    StringArena arena(&alloc);

    for (auto _ : state) {
        for (int i = 0; i < 16; ++i) {
            auto s = arena.allocate();
            benchmark::DoNotOptimize(s);
        }
    }
}
BENCHMARK(BM_ContentionStress)->ThreadRange(1, 64);

static void BM_BurstAllocation(benchmark::State& state)
{
    auto alloc = Allocation(detail::STRING_ALLOCATION_SIZE, 0);
    StringArena arena(&alloc);

    for (auto _ : state) {
        std::vector<NUMAString> batch;
        batch.reserve(32);

        for (int i = 0; i < 32; ++i) {
            batch.emplace_back(arena.allocate());
        }

        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_BurstAllocation);

static void BM_CapacityPressure(benchmark::State& state)
{
    auto alloc = Allocation(detail::STRING_ALLOCATION_SIZE, 0);
    StringArena arena(&alloc);

    std::vector<NUMAString> v;
    v.reserve(detail::STRING_ARENA_SIZE);
    for (auto _ : state) {
        v.clear();
        
        for (std::size_t i = 0; i < detail::STRING_ARENA_SIZE; ++i) {
            v.emplace_back(arena.allocate());
        }

        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_CapacityPressure);

static void BM_StdString(benchmark::State& state)
{
    for (auto _ : state) {
        std::string s;
        s.reserve(256);

        for (int i = 0; i < 32; ++i) {
            s.push_back('a');
        }

        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(BM_StdString);

BENCHMARK_MAIN();