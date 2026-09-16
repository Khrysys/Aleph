/**
 * @file tests/fuzz_board_constructor.cpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <cstdint>
#include <stdexcept>
#include <string_view>

#include <aleph/chess/board.hpp>

using namespace aleph::chess;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size == 0) return 0;

    std::string_view fen(reinterpret_cast<const char*>(data), size);

    try {
        Board b(fen);

        // If construction succeeded, verify basic invariants
        ASSERT(aleph::platform::popcnt(b.getWhiteOccupancy() & b.getBlackOccupancy()) == 0);
        ASSERT(aleph::platform::popcnt(b.getOccupancy()) ==
               aleph::platform::popcnt(b.getWhiteOccupancy()) +
                   aleph::platform::popcnt(b.getBlackOccupancy()));
        ASSERT(aleph::platform::popcnt(b.getBlackOccupancy()) >= 1);
        ASSERT(aleph::platform::popcnt(b.getWhiteOccupancy()) >= 1);

    } catch (const std::invalid_argument&) {
        // Expected for malformed FEN — not a bug
    }

    return 0;
}