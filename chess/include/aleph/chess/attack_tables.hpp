/**
 * @file include/aleph/chess/attack_tables.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <array>
#include <cstdint>

#include "piece.hpp"
#include "square.hpp"

namespace aleph::chess {
    namespace detail {
        /**
         * Precomputed attack and between tables for all piece types and squares.
         *
         * Generated at build time by `chess/tools/table_builder.py` and stored in
         * `generated_tables.inl` as a `constinit` definition. Do not construct this
         * struct directly — access it via the `attackTables` global in `generated_tables.inl`.
         *
         * `movement` is indexed as `movement[pieceIndex][square]`, where piece indices
         * follow the `Piece` encoding: white pieces at 0..5 and black pieces at 6..11,
         * matching `PieceType` values directly for white and offset by 6 for black.
         * Pawn tables are asymmetric and encode capture attacks only, not pushes.
         * Slider tables (bishop, rook, queen) are unblocked full rays — callers must
         * mask against occupancy and use the `between` table to validate path clarity.
         *
         * `between` is indexed as `between[from][to]` and gives a bitboard of squares
         * strictly between `from` and `to` along the shared rank, file, or diagonal.
         * Returns zero if the squares are not aligned or are adjacent. Used by
         * `isLegalFast` to validate slider paths and detect discovered checks.
         */
        struct AttackTables {
                /**
                 * Attack masks for each piece type and square, indexed as
                 * `movement[pieceIndex][square]`.
                 *
                 * Piece indices follow the `Piece` uint8_t encoding: white PAWN=0 through white
                 * KING=5, black PAWN=6 through black KING=11. Use `Piece(type, isBlack)` as an
                 * index to select the correct table for a given color, exploiting the implicit
                 * `operator uint8_t()` conversion. Pawn entries encode diagonal capture attacks
                 * only — push generation is handled separately in `getPseudoLegalMoves`. Slider
                 * entries are unblocked full rays in all relevant directions; path validity must be
                 * checked by the caller using the `between` table and current board occupancy.
                 */
                std::array<std::array<uint64_t, 64>, 12> movement;

                /**
                 * Squares strictly between two squares, indexed as `between[from][to]`.
                 *
                 * Returns a bitboard of all squares lying strictly between `from` and `to`
                 * along their shared rank, file, or diagonal. Returns zero if the squares
                 * are not aligned on any of those axes, or if they are adjacent. The endpoint
                 * squares themselves are never included. Used to validate slider move paths
                 * and to compute block masks for check evasion in `isLegalFast`.
                 */
                std::array<std::array<uint64_t, 64>, 64> between;
                /**
                 * Squares strictly in a direction from a square, indexed as
                 * `rays[direction][from]`.
                 *
                 * Returns a bitboard of all squares lying strictly in a direction from `from` in a
                 * direction (See `aleph::chess::detail::Direction`). `from` itself is not included.
                 * Used to validate sliding pieces for check detection.
                 */
                std::array<std::array<uint64_t, 64>, 8> rays;
        };
    }  // namespace detail
}  // namespace aleph::chess

#include <aleph/chess/generated_tables.inl>