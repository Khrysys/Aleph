/**
 * @file include/aleph/chess/square.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <cstdint>
#include <string>

#include <fmt/format.h>
#include <libassert/assert.hpp>

#include <aleph/platform.hpp>

namespace aleph::chess {

    /** Character lookup tables for algebraic notation formatting. */
    namespace detail {
        constexpr std::string_view FILE_CHARS = "abcdefgh";
        constexpr std::string_view RANK_CHARS = "12345678";
    }  // namespace detail

    /**
     * Represents a single square on the chessboard as a compact index in [0, 63].
     *
     * Squares are indexed row-major from a1 (0) to h8 (63), where rank increases
     * with the index and file increases within each rank. The index layout is:
     *
     *   index = rank * 8 + file
     *
     * where rank 0 is the first rank (white's back rank) and file 0 is the a-file.
     * Instances are implicitly convertible to `uint8_t` for use as bitboard shift
     * amounts and array indices.
     */
    class Square {
        public:
            /** Constructs a1 (index 0) by default. */
            constexpr Square() : data(0) {}

            /**
             * Constructs a square from a raw index in [0, 63].
             * Asserts that `d` is a valid square index in debug builds.
             */
            constexpr Square(std::uint8_t d) : data(d) { DEBUG_ASSERT(d < 64); }

            /**
             * Constructs a square from a rank and file, both in [0, 7].
             * Asserts that both `r` and `f` are in range in debug builds.
             */
            constexpr Square(std::uint8_t r, std::uint8_t f) : data(r * 8 + f) {
                DEBUG_ASSERT(r < 8);
                DEBUG_ASSERT(f < 8);
            }

            /** Returns the rank of this square in [0, 7], where 0 is the first rank. */
            [[nodiscard]] constexpr inline std::uint8_t rank() const noexcept { return data >> 3; }

            /** Returns the file of this square in [0, 7], where 0 is the a-file. */
            [[nodiscard]] constexpr inline std::uint8_t file() const noexcept { return data & 7; }

            /**
             * Returns the algebraic notation for this square, e.g. "e4".
             * The returned string is always exactly two characters: file letter followed by rank
             * digit.
             */
            [[nodiscard]]
#ifdef ALEPH_CONSTEXPR_STRING
            constexpr
#endif
                inline std::string toString() const noexcept {
                return {detail::FILE_CHARS[file()], detail::RANK_CHARS[rank()]};
            }

            /** Implicit conversion to `uint8_t` for use as a bitboard shift amount or array index.
             */
            constexpr inline operator std::uint8_t() const noexcept { return data; }

        private:
            std::uint8_t data;
    };

    /**
     * Represents a cardinal or diagonal step direction as a signed square index delta.
     *
     * These values can be added to a square index to step in the given direction,
     * provided the result remains on the board. No wraparound checking is performed
     * by the direction itself.
     */
    enum Direction : int8_t {
        TOWARDS_A_FILE = -1,  ///< One step toward the a-file (decreasing file).
        TOWARDS_H_FILE = 1,   ///< One step toward the h-file (increasing file).
        TOWARDS_1_RANK = -8,  ///< One step toward the first rank (decreasing rank).
        TOWARDS_8_RANK = 8    ///< One step toward the eighth rank (increasing rank).
    };

}  // namespace aleph::chess

/// @cond INTERNAL
/**
 * Formats a `Square` as its algebraic notation (e.g. "e4") for use with spdlog and fmtlib.
 * Registered outside `aleph::chess` per fmtlib specialization requirements.
 */
template <>
struct fmt::formatter<aleph::chess::Square> {
        constexpr auto parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

        auto format(const aleph::chess::Square& sq, fmt::format_context& ctx) const {
            return fmt::format_to(ctx.out(), "{}", sq.toString());
        }
};
/// @endcond