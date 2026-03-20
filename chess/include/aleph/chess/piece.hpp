/**
 * @file include/aleph/chess/piece.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <cstdint>
#include <locale>
#include <string_view>

#include <fmt/format.h>
#include <libassert/assert.hpp>

namespace aleph::chess {

    /**
     * Identifies the type of a chess piece, independent of color.
     *
     * Values 0-5 map directly to bitboard array indices and move table indices,
     * so the numeric values are load-bearing and must not be changed. `NONE`
     * is a sentinel used when no piece occupies a square; it is intentionally
     * set to 0x80 to avoid colliding with any valid index.
     */
    enum PieceType : std::uint8_t {
        PAWN   = 0,
        BISHOP = 1,
        KNIGHT = 2,
        ROOK   = 3,
        QUEEN  = 4,
        KING   = 5,
        NONE   = 0x80
    };

    namespace detail {
        constexpr std::string_view PIECE_TYPE_CHARS = "PBNRQKpbnrqk";
    }

    /**
     * Represents a chess piece as a compact index encoding both type and color.
     *
     * The encoding is `type + (6 * isBlack)`, mapping white pieces to [0, 5]
     * and black pieces to [6, 11]. This matches the layout of the move table
     * and bitboard arrays in `Board`, so the raw `uint8_t` value can be used
     * directly as an array index.
     *
     * `NONE` is not a valid piece type for construction and is rejected by
     * assertion in debug builds.
     */
    class Piece {
        public:
            /**
             * Constructs a piece from a type and color.
             * Asserts that `type` is not `NONE` in debug builds.
             */
            constexpr Piece(PieceType type, bool isBlack)
                : data(static_cast<uint8_t>(type + (6 * isBlack))) {
                DEBUG_ASSERT(type != NONE);
            }

            /**
             * Constructs a piece from its FEN character representation.
             * Uppercase is white, lowercase is black. Asserts on unrecognized characters
             * in debug builds.
             */
            constexpr explicit Piece(char c) : data(0) {
                auto pos = detail::PIECE_TYPE_CHARS.find(c);
                DEBUG_ASSERT(pos != std::string_view::npos);
                data = static_cast<uint8_t>(pos);
            }

            /** Returns the type of this piece, independent of color. */
            [[nodiscard]] constexpr inline PieceType type() const noexcept {
                return static_cast<PieceType>(data % 6);
            }

            /** Returns true if this piece belongs to the black side. */
            [[nodiscard]] constexpr inline bool isBlack() const noexcept { return data >= 6; }
            /** Returns true if this piece belongs to the white side. */
            [[nodiscard]] constexpr inline bool isWhite() const noexcept { return !isBlack(); }

            /**
             * Returns the lowercase FEN character for this piece type, e.g. 'p', 'n', 'k'.
             * Use `isupper` / `toupper` at the display layer for white pieces.
             */
            [[nodiscard]] constexpr inline char toChar() const noexcept {
                return detail::PIECE_TYPE_CHARS[data];
            }

            /** Implicit conversion to `uint8_t` for use as a bitboard or move table index. */
            constexpr inline operator std::uint8_t() const noexcept { return data; }

        private:
            std::uint8_t data;
    };

}  // namespace aleph::chess

/// @cond INTERNAL
/**
 * Formats a `Piece` as its FEN character representation: uppercase for white,
 * lowercase for black (e.g. "P", "n", "K"). Registered outside `aleph::chess`
 * per fmtlib specialization requirements.
 */
template <>
struct fmt::formatter<aleph::chess::Piece> {
        constexpr auto parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

        auto format(const aleph::chess::Piece& p, fmt::format_context& ctx) const {
            return fmt::format_to(ctx.out(), "{}", p.toChar());
        }
};
/// @endcond