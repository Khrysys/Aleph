/**
 * @file tests/bench_allocation.cpp
 * 
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
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
    void BM_SubAllocationAccess(benchmark::State& state) {
        Allocation alloc(2048, 0);
        auto sub = alloc.getSubAllocation<int>(2048);

        for (auto _ : state) {
            for (std::size_t i = 0; i < sub.getSize(); ++i) {
                sub[i] = static_cast<int>(i);
            }
        }
    }
    BENCHMARK(BM_SubAllocationAccess);

    void BM_Allocate_SingleThread(benchmark::State& state) {
        auto alloc = Allocation(detail::STRING_ALLOCATION_SIZE, 0);
        StringArena arena(&alloc);

        for (auto _ : state) {
            auto s = arena.allocate();
            benchmark::DoNotOptimize(s);
        }
    }
    BENCHMARK(BM_Allocate_SingleThread);

    void BM_AllocateAndWrite(benchmark::State& state) {
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

    void BM_AllocateDestroy(benchmark::State& state) {
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

    void BM_CopyNUMAString(benchmark::State& state) {
        auto alloc = Allocation(detail::STRING_ALLOCATION_SIZE, 0);
        StringArena arena(&alloc);
        auto base = arena.allocate();

        for (auto _ : state) {
            auto copy = base;
            benchmark::DoNotOptimize(copy);
        }
    }
    BENCHMARK(BM_CopyNUMAString);

    void BM_AllocateFree_MultiThread(benchmark::State& state) {
        static Allocation* alloc  = nullptr;
        static StringArena* arena = nullptr;

        if (state.thread_index() == 0) {
            alloc = new Allocation(detail::STRING_ALLOCATION_SIZE, 0);
            arena = new StringArena(alloc);
        }

        for (auto _ : state) {
            auto s = arena->allocate();
            benchmark::DoNotOptimize(s);
        }

        if (state.thread_index() == 0) {
            delete arena;
            delete alloc;
        }
    }
    BENCHMARK(BM_AllocateFree_MultiThread)->ThreadRange(1, 64);

    void BM_ContentionStress(benchmark::State& state) {
        static Allocation* alloc  = nullptr;
        static StringArena* arena = nullptr;

        if (state.thread_index() == 0) {
            alloc = new Allocation(detail::STRING_ALLOCATION_SIZE, 0);
            arena = new StringArena(alloc);
        }

        for (auto _ : state) {
            for (int i = 0; i < 16; ++i) {
                auto s = arena->allocate();
                benchmark::DoNotOptimize(s);
            }
        }

        if (state.thread_index() == 0) {
            delete arena;
            delete alloc;
        }
    }
    BENCHMARK(BM_ContentionStress)->ThreadRange(1, 64);
    void BM_BurstAllocation(benchmark::State& state) {
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

    void BM_CapacityPressure(benchmark::State& state) {
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

    void BM_StdString(benchmark::State& state) {
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
}  // namespace

BENCHMARK_MAIN();