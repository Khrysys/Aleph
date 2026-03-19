#pragma once

#include <cstdint>
#include <string>

#include <fmt/format.h>
#include <libassert/assert.hpp>

#include "piece.hpp"
#include "square.hpp"

namespace aleph::chess {

    /**
     * Represents a chess move as a compact 16-bit value encoding the from square,
     * to square, and an optional promotion piece type.
     *
     * Bit layout:
     *   [5:0]   from square index
     *   [11:6]  to square index
     *   [15:12] promotion piece type (0 = no promotion)
     *
     * Castling and en passant are not encoded explicitly — they are inferred
     * contextually by `Board::push()` from the from/to squares and board state.
     * The implicit conversion to `uint16_t` allows direct use as a policy table index.
     */
    class Move {
        public:
            /** Constructs a non-promotion move. */
            constexpr inline Move(Square from, Square to) : data(from | (to << 6)) {}

            /** Constructs a promotion move with the given promotion piece type. */
            constexpr inline Move(Square from, Square to, PieceType promo)
                : data(from | (to << 6) | (promo << 12)) {
                DEBUG_ASSERT(promo != PieceType::NONE);
                DEBUG_ASSERT(promo != PieceType::KING);
            }

            /** Constructs a default move that is null (a1a1) and has no promotion. */
            constexpr inline Move() : data(0) {}

            /** Returns the origin square of this move. */
            [[nodiscard]] constexpr inline Square from() const noexcept { return data & 0x003F; }

            /** Returns the destination square of this move. */
            [[nodiscard]] constexpr inline Square to() const noexcept {
                return (data >> 6) & 0x003F;
            }

            /**
             * Returns the promotion piece type, or `PAWN` (0) if no promotion is encoded.
             * Use `hasPromo()` to distinguish a pawn promotion from a non-promotion move.
             */
            [[nodiscard]] constexpr inline PieceType promo() const noexcept {
                return PieceType((data >> 12) & 0x000F);
            }

            /**
             * Returns true if this move encodes a promotion.
             * A promotion is indicated by a non-zero promotion field, i.e. any piece
             * type other than `PAWN` (0).
             */
            [[nodiscard]] constexpr inline bool hasPromo() const noexcept {
                return (data >> 12) != 0;
            }

            /** Returns this move in UCI notation, e.g. "e2e4" or "e7e8q". */
            [[nodiscard]] constexpr inline std::string toString() const {
                std::string r = from().toString() + to().toString();
                if (hasPromo()) r += detail::PIECE_TYPE_CHARS[promo() + 6];
                return r;
            }

            /** Implicit conversion to `uint16_t` for use as a policy table index. */
            constexpr inline operator std::uint16_t() const noexcept { return data; }

        private:
            std::uint16_t data;
    };

}  // namespace aleph::chess

/**
 * Formats a `Move` in UCI notation, e.g. "e2e4" or "e7e8q".
 * Promotion piece is appended as a lowercase character when present.
 * Registered outside `aleph::chess` per fmtlib specialization requirements.
 */
template <>
struct fmt::formatter<aleph::chess::Move> {
        constexpr auto parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

        auto format(const aleph::chess::Move& m, fmt::format_context& ctx) const {
            return fmt::format_to(ctx.out(), "{}", m.toString());
        }
};