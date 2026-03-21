#include <cstdint>

#include <aleph/chess/board.hpp>

using namespace aleph::chess;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 1) return 0;

    Board b;

    for (size_t i = 0; i < size; i++) {
        auto moves = b.getLegalMoves();
        if (moves.empty()) break;

        b = b.push(moves[data[i] % moves.size()]);

        ASSERT(aleph::platform::popcnt(b.whiteOccupancy() & b.blackOccupancy()) == 0);
        ASSERT(aleph::platform::popcnt(b.occupancy()) ==
               aleph::platform::popcnt(b.whiteOccupancy()) +
                   aleph::platform::popcnt(b.blackOccupancy()));
        ASSERT(aleph::platform::popcnt(b.blackOccupancy() >= 1));
        ASSERT(aleph::platform::popcnt(b.whiteOccupancy() >= 1));
    }

    return 0;
}