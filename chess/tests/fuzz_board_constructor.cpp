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
        ASSERT(aleph::platform::popcnt(b.whiteOccupancy() & b.blackOccupancy()) == 0);
        ASSERT(aleph::platform::popcnt(b.occupancy()) ==
               aleph::platform::popcnt(b.whiteOccupancy()) + aleph::platform::popcnt(b.blackOccupancy()));
        ASSERT(aleph::platform::popcnt(b.blackOccupancy()) >= 1);
        ASSERT(aleph::platform::popcnt(b.whiteOccupancy()) >= 1);

    } catch (const std::invalid_argument&) {
        // Expected for malformed FEN — not a bug
    }

    return 0;
}