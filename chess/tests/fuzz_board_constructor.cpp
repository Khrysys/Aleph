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
        ASSERT(std::popcount(b.whiteOccupancy() & b.blackOccupancy()) == 0);
        ASSERT(std::popcount(b.occupancy()) ==
               std::popcount(b.whiteOccupancy()) + std::popcount(b.blackOccupancy()));
        ASSERT(std::popcount(b.blackOccupancy() >= 1));
        ASSERT(std::popcount(b.whiteOccupancy() >= 1));

    } catch (const std::invalid_argument&) {
        // Expected for malformed FEN — not a bug
    }

    return 0;
}