/**
 * @file tests/fuzz_board_push.cpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <cstdint>

#include <aleph/chess/board.hpp>

using namespace aleph::chess;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 1) return 0;

    Board b{};

    for (size_t i = 0; i < size; i++) {
        auto moves = b.getLegalMoves();
        if (moves.empty()) break;

        b = b.push(moves[data[i] % moves.size()]);

        ASSERT(aleph::platform::popcnt(b.getWhiteOccupancy() & b.getBlackOccupancy()) == 0);
        ASSERT(aleph::platform::popcnt(b.getOccupancy()) ==
               aleph::platform::popcnt(b.getWhiteOccupancy()) +
                   aleph::platform::popcnt(b.getBlackOccupancy()));
        ASSERT(aleph::platform::popcnt(b.getBlackOccupancy() >= 1));
        ASSERT(aleph::platform::popcnt(b.getWhiteOccupancy() >= 1));
    }

    return 0;
}