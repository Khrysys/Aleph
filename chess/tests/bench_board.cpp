/**
 * @file tests/bench_board.cpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <benchmark/benchmark.h>

#include <aleph/chess/board.hpp>

using namespace aleph::chess;

static uint64_t perft(Board& board, int depth) {
    auto moves = board.getLegalMoves();

    if (depth == 1) return moves.size();

    uint64_t nodes = 0;
    for (const auto& move : moves) {
        Board next  = board.push(move);
        nodes      += perft(next, depth - 1);
    }
    return nodes;
}

static const std::unordered_map<std::string, std::unordered_map<int, uint64_t>> EXPECTED_NODES = {
    {               "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
     {{4, 197281}, {5, 4865609}, {6, 119060324}, {7, 3195901860}}},
    {       "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
     {{4, 4085603}, {5, 193690690}, {6, 8031647685}}             },
    {                                  "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
     {
     {4, 43238},
     {5, 674624},
     }                                                           },
    {       "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
     {
     {4, 422333},
     {5, 15833292},
     }                                                           },
    {              "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
     {
     {4, 2103487},
     {5, 89941194},
     }                                                           },
    {"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 1",
     {
     {4, 3894594},
     {5, 164075551},
     }                                                           },
};

static void runPerft(benchmark::State& state, const char* fen) {
    Board board(fen);
    uint64_t nodes = 0;

    // NOLINTNEXTLINE
    for (auto _ : state) nodes = perft(board, state.range(0));

    state.counters["nodes"] =
        benchmark::Counter(static_cast<double>(nodes), benchmark::Counter::kDefaults);

    state.counters["nps"] =
        benchmark::Counter(static_cast<double>(nodes), benchmark::Counter::kIsRate);

    state.counters["depth"] = static_cast<double>(state.range(0));

    // Verify correctness
    auto fenIt              = EXPECTED_NODES.find(fen);
    if (fenIt != EXPECTED_NODES.end()) {
        auto depthIt = fenIt->second.find(static_cast<int>(state.range(0)));
        if (depthIt != fenIt->second.end()) {
            if (nodes != depthIt->second) {
                state.SkipWithError(("Perft mismatch: got " + std::to_string(nodes) + " expected " +
                                     std::to_string(depthIt->second))
                                        .c_str());
            }
        }
    }
}

static void BM_PerftStarting(benchmark::State& state) {
    runPerft(state, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}
BENCHMARK(BM_PerftStarting)->Unit(benchmark::kMillisecond)->Arg(4)->Arg(5)->Arg(6);

static void BM_PerftKiwipete(benchmark::State& state) {
    runPerft(state, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
}
BENCHMARK(BM_PerftKiwipete)->Unit(benchmark::kMillisecond)->Arg(4)->Arg(5)->Arg(6);

static void BM_PerftPosition3(benchmark::State& state) {
    runPerft(state, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");
}
BENCHMARK(BM_PerftPosition3)->Unit(benchmark::kMillisecond)->Arg(4)->Arg(5);

static void BM_PerftPosition4(benchmark::State& state) {
    runPerft(state, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
}
BENCHMARK(BM_PerftPosition4)->Unit(benchmark::kMillisecond)->Arg(4)->Arg(5);

static void BM_PerftPosition5(benchmark::State& state) {
    runPerft(state, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
}
BENCHMARK(BM_PerftPosition5)->Unit(benchmark::kMillisecond)->Arg(4)->Arg(5);

static void BM_PerftPosition6(benchmark::State& state) {
    runPerft(state, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 1");
}
BENCHMARK(BM_PerftPosition6)->Unit(benchmark::kMillisecond)->Arg(4)->Arg(5);

BENCHMARK_MAIN();