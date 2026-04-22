#include <cstddef>
#include <string>
#include <thread>
#include <vector>

#include <benchmark/benchmark.h>

#include <aleph/platform.hpp>

using namespace aleph::platform::allocation;

namespace {
    // -----------------------------
    // Allocation benchmark
    // -----------------------------
    void BM_AllocationConstruction(benchmark::State& state) {
        for (auto _ : state) {
            Allocation alloc(4096, 0);
            benchmark::DoNotOptimize(alloc);
        }
    }
    BENCHMARK(BM_AllocationConstruction);

    // -----------------------------
    // Large allocation benchmark
    // -----------------------------
    void BM_LargeAllocation(benchmark::State& state) {
        for (auto _ : state) {
            Allocation alloc(1 << 28, 0);  // 128MB
            benchmark::DoNotOptimize(alloc);
        }
    }
    BENCHMARK(BM_LargeAllocation);

    // -----------------------------
    // SubAllocation access benchmark
    // -----------------------------
    void BM_SubAllocationIteration(benchmark::State& state) {
        Allocation alloc(2048, 0);
        auto sub = alloc.getSubAllocation<int>(2048);

        for (auto _ : state) {
            for (std::size_t i = 0; i < sub.getSize(); ++i) {
                sub[i] = static_cast<int>(i);
            }
        }
    }
    BENCHMARK(BM_SubAllocationIteration)->Range(1 << 8, 1 << 24);
}

BENCHMARK_MAIN();