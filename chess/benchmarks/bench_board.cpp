/**
 * @file tests/bench_board.cpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <vector>

#include <benchmark/benchmark.h>

#include <aleph/chess.hpp>

static std::vector<aleph::chess::Board> loadPositions() {
    using namespace aleph::chess;

    return {
        Board{},
        Board{"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"},
        Board{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -"},
        Board{"r4rk1/1pp1qppp/p1npbn2/8/2BPP3/2N1BN2/PPP2PPP/R2Q1RK1 w - -"},
    };
}

static void BM_isAttackedBy(benchmark::State& state) {
    using namespace aleph::chess;

    static auto positions = loadPositions();
    size_t idx            = 0;

    for (auto _ : state) {
        std::array<uint64_t, 6> enemy{};
        auto& b        = positions[idx++ % positions.size()];

        bool blackTurn = b.isBlackTurn();
        enemy          = blackTurn ? b.getWhiteBitboards() : b.getBlackBitboards();

        uint64_t occ   = b.getOccupancy();

        uint8_t sq     = static_cast<uint8_t>(idx % 64);

        benchmark::DoNotOptimize(detail::isAttackedBy(sq, occ, enemy, !blackTurn));
    }
}
BENCHMARK(BM_isAttackedBy);

static void BM_isLegalFast(benchmark::State& state) {
    using namespace aleph::chess;

    static auto positions = loadPositions();
    size_t idx            = 0;

    for (auto _ : state) {
        auto& b    = positions[idx++ % positions.size()];

        auto moves = b.getPseudoLegalMoves();

        for (const auto& m : moves) {
            benchmark::DoNotOptimize(b.isLegalFast(m));
        }
    }
}
BENCHMARK(BM_isLegalFast);

static void BM_push(benchmark::State& state) {
    using namespace aleph::chess;

    static auto positions = loadPositions();
    size_t idx            = 0;

    for (auto _ : state) {
        auto& b    = positions[idx++ % positions.size()];

        auto moves = b.getPseudoLegalMoves();

        for (const auto& m : moves) {
            if (b.isLegalFast(m)) {
                auto next = b.push(m);
                benchmark::DoNotOptimize(next);
            }
        }
    }
}
BENCHMARK(BM_push);

static void BM_getLegalMoves(benchmark::State& state) {
    using namespace aleph::chess;

    static auto positions = loadPositions();
    size_t idx            = 0;

    for (auto _ : state) {
        auto& b    = positions[idx++ % positions.size()];

        auto moves = b.getLegalMoves();
        benchmark::DoNotOptimize(moves);
    }
}
BENCHMARK(BM_getLegalMoves);

static void BM_perft_node(benchmark::State& state) {
    using namespace aleph::chess;

    static auto positions = loadPositions();
    size_t idx            = 0;

    for (auto _ : state) {
        auto& b    = positions[idx++ % positions.size()];

        auto moves = b.getLegalMoves();

        for (const auto& m : moves) {
            auto next = b.push(m);
            benchmark::DoNotOptimize(next);
        }
    }
}
BENCHMARK(BM_perft_node);

static void BM_bishopAttacks(benchmark::State& state) {
    using namespace aleph::chess;

    static auto positions = loadPositions();
    size_t idx            = 0;

    for (auto _ : state) {
        auto& b      = positions[idx++ % positions.size()];
        uint64_t occ = b.getOccupancy();

        uint8_t sq   = idx % 64;

        benchmark::DoNotOptimize(detail::bishopAttacks(sq, occ));
    }
}
BENCHMARK(BM_bishopAttacks);

BENCHMARK_MAIN();