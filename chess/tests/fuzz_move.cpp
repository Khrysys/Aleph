#include <cstdint>

#include <libassert/assert.hpp>

#include <aleph/chess/move.hpp>

using namespace aleph::chess;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 2) return 0;

    uint8_t from = data[0] & 0x3F;
    uint8_t to   = data[1] & 0x3F;

    Move m(Square(from), Square(to));
    ASSERT(static_cast<uint8_t>(m.from()) == from);
    ASSERT(static_cast<uint8_t>(m.to()) == to);
    ASSERT(!m.hasPromo());

    if (size >= 3) {
        static constexpr PieceType VALID_PROMOS[] = {BISHOP, KNIGHT, ROOK, QUEEN};
        PieceType promo                           = VALID_PROMOS[data[2] % 4];
        Move mp(Square(from), Square(to), promo);
        ASSERT(static_cast<uint8_t>(mp.from()) == from);
        ASSERT(static_cast<uint8_t>(mp.to()) == to);
        ASSERT(mp.hasPromo());
        ASSERT(mp.promo() == promo);
    }

    return 0;
}