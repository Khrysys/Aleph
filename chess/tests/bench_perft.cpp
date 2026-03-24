/**
 * @file tests/bench_perft.cpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <string>
#include <tuple>

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

template <class... Args>
static void BM_Perft(benchmark::State& state, Args&&... args) {
    auto argsTuple   = std::make_tuple(std::move(args)...);
    auto fen         = std::get<0>(argsTuple);
    auto depth       = std::get<1>(argsTuple);
    auto targetNodes = std::get<2>(argsTuple);

    uint64_t nodes   = 0;

    Board board{fen};
    for (auto _ : state) {
        // cppcheck-suppress[useStlAlgorithm]
        nodes = perft(board, depth);
    }
    if (nodes != targetNodes) {
        state.SkipWithError(("Perft mismatch: got " + std::to_string(nodes) + " expected " +
                             std::to_string(targetNodes))
                                .c_str());
    }
    state.counters["nodes"] = nodes;
    state.counters["nps"]   = benchmark::Counter(static_cast<double>(nodes),
                                                 benchmark::Counter::kIsIterationInvariantRate);
}

// clang-format off
// {FEN, Depth, Nodes}
BENCHMARK_CAPTURE(BM_Perft, Pos1Depth1, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",                 1,         20)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Pos1Depth2, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",                 2,        400)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Pos1Depth3, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",                 3,       8902)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Pos1Depth4, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",                 4,     197281)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Perft, Pos1Depth5, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",                 5,    4865609)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Perft, Pos1Depth6, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",                 6,  119060324)->Unit(benchmark::kMillisecond);
//BENCHMARK_CAPTURE(BM_Perft, Pos1Depth7, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",                 7, 3195901860)->Unit(benchmark::kMillisecond);

BENCHMARK_CAPTURE(BM_Perft, Pos2Depth1, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ",        1,         48)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Pos2Depth2, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ",        2,       2039)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Pos2Depth3, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ",        3,      97862)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Pos2Depth4, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ",        4,    4085603)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Perft, Pos2Depth5, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ",        5,  193690690)->Unit(benchmark::kMillisecond);
//BENCHMARK_CAPTURE(BM_Perft, Pos2Depth6, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ",        6, 8031647685)->Unit(benchmark::kMillisecond);

BENCHMARK_CAPTURE(BM_Perft, Pos3Depth1, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",                                1,         14)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Pos3Depth2, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",                                2,        191)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Pos3Depth3, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",                                3,       2812)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Pos3Depth4, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",                                4,      43238)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Perft, Pos3Depth5, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",                                5,     674624)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Perft, Pos3Depth6, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",                                6,   11030083)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Perft, Pos3Depth7, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",                                7,  178633661)->Unit(benchmark::kMillisecond);
//BENCHMARK_CAPTURE(BM_Perft, Pos3Depth8, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",                                8, 3009794393)->Unit(benchmark::kMillisecond);

BENCHMARK_CAPTURE(BM_Perft, Pos4Depth1, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",         1,          6)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Pos4Depth2, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",         2,        264)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Pos4Depth3, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",         3,       9467)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Pos4Depth4, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",         4,     422333)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Perft, Pos4Depth5, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",         5,   15833292)->Unit(benchmark::kMillisecond);
//BENCHMARK_CAPTURE(BM_Perft, Pos4Depth6, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",         6,  706045033)->Unit(benchmark::kMillisecond);

BENCHMARK_CAPTURE(BM_Perft, Po4MDepth1, "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",         1,          6)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Po4MDepth2, "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",         2,        264)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Po4MDepth3, "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",         3,       9467)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Po4MDepth4, "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",         4,     422333)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Perft, Po4MDepth5, "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",         5,   15833292)->Unit(benchmark::kMillisecond);
//BENCHMARK_CAPTURE(BM_Perft, Po4MDepth6, "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",         6,  706045033)->Unit(benchmark::kMillisecond);

BENCHMARK_CAPTURE(BM_Perft, Pos5Depth1, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",                1,         44)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Pos5Depth2, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",                2,       1486)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Pos5Depth3, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",                3,      62379)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Pos5Depth4, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",                4,    2103487)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Perft, Pos5Depth5, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",                5,   89941194)->Unit(benchmark::kMillisecond);
//BENCHMARK_CAPTURE(BM_Perft, Pos5Depth6, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",                6, 3048196529)->Unit(benchmark::kMillisecond);

BENCHMARK_CAPTURE(BM_Perft, Pos6Depth1, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 1,         46)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Pos6Depth2, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 2,       2079)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Pos6Depth3, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 3,      89890)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Perft, Pos6Depth4, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 4,    3894594)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Perft, Pos6Depth5, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 5,  164075551)->Unit(benchmark::kMillisecond);
//BENCHMARK_CAPTURE(BM_Perft, Pos6Depth6, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 6, 6923051137)->Unit(benchmark::kMillisecond);
//clang-format on

BENCHMARK_MAIN();