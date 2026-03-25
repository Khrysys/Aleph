/**
 * @file include/aleph/chess/board.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

#include <libassert/assert.hpp>

#include <aleph/platform.hpp>

#include "move.hpp"
#include "move_list.hpp"
#include "piece.hpp"
#include "square.hpp"

namespace aleph::chess {
    namespace detail {
        /** FEN string corresponding to the traditional starting chess position. */
        constexpr std::string_view STARTING_POSITION_FEN =
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    }  // namespace detail

    /**
     * Encodes the fields packed into the 32-bit `metadata` word of `Board`.
     *
     * Layout:
     *   [2:0]   EN_PASSANT_FILE_MASK   — file index (0-7) of the en passant target square.
     *   [3]     EN_PASSANT_VALID       — set if an en passant capture is currently available.
     *   [4]     BLACK_TO_MOVE          — set if it is black's turn to move.
     *   [5]     WHITE_KINGSIDE_CASTLE  — set if white retains kingside castling rights.
     *   [6]     WHITE_QUEENSIDE_CASTLE — set if white retains queenside castling rights.
     *   [7]     BLACK_KINGSIDE_CASTLE  — set if black retains kingside castling rights.
     *   [8]     BLACK_QUEENSIDE_CASTLE — set if black retains queenside castling rights.
     *   [15:9]  HALF_MOVE_CLOCK        — halfmove clock for the 50-move rule (0-100).
     *                                    Extract via `(metadata & HALF_MOVE_CLOCK) >> 9`.
     *   [31:16] — unused, must be zero.
     */
    enum BoardMetadataFlags : uint32_t {
        EN_PASSANT_FILE_MASK   = 0x00000007,
        EN_PASSANT_VALID       = 0x00000008,
        BLACK_TO_MOVE          = 0x00000010,
        WHITE_KINGSIDE_CASTLE  = 0x00000020,
        WHITE_QUEENSIDE_CASTLE = 0x00000040,
        BLACK_KINGSIDE_CASTLE  = 0x00000080,
        BLACK_QUEENSIDE_CASTLE = 0x00000100,
        HALF_MOVE_CLOCK        = 0x0000FE00,
        CACHED_CHECKERS_VALID  = 0x80000000
    };

    /**
     * Represents a chess position as an immutable value type.
     *
     * The board state is encoded as twelve 64-bit bitboards (six per side, one per
     * piece type), a 32-bit metadata word encoding side-to-move, castling rights,
     * en passant state, and the halfmove clock. See `BoardMetadataFlags` for the
     * exact bit layout.
     *
     * Positions are mutated exclusively via `push()`, which returns a new `Board`
     * with the move applied. The original board is never modified. Mutable fields
     * (`_cacheValid`, `_occupancy`, etc.) exist solely for lazy recomputation of
     * derived values and do not affect observable board state — the type behaves
     * as a value type for all practical purposes.
     *
     * Castling and en passant are detected contextually in `push()` rather than
     * encoded in move bits — a king moving two squares triggers rook relocation,
     * and a pawn capturing onto the en passant square triggers removal of the
     * captured pawn. This keeps `Move` representation minimal at the cost of
     * slightly more inference per `push()` call.
     *
     * In debug builds, `push()` asserts `isLegal(m)` before applying the move.
     * `isLegalFast()` must never call `push()` to avoid infinite recursion through
     * this assert.
     */
    class Board {
        public:
            /**
             * Constructs the standard chess starting position.
             * Equivalent to `Board(detail::STARTING_POSITION_FEN)`.
             */
            inline Board();

            /**
             * Constructs a board from a FEN string.
             *
             * Parses all six FEN fields, validating piece placement, side to move,
             * castling rights, en passant square, halfmove clock, and fullmove number.
             * Validation is strict — any inconsistency between fields (e.g. castling
             * rights claimed when the king or rook is absent, en passant square on the
             * wrong rank, the non-moving side in check) throws `std::invalid_argument`
             * with a descriptive message. The fullmove number is validated but not stored.
             * A minimum of four fields is required; fields five and six are optional.
             *
             * Throws `std::invalid_argument` on any validation failure.
             */
            inline Board(std::string_view fen);

            /**
             * Returns the piece occupying the given square, or a `Piece` with type
             * `NONE` if the square is empty. Checks occupancy first to avoid scanning
             * all twelve bitboards on empty squares.
             */
            [[nodiscard]] inline Piece get(Square sq) const;

            /**
             * Returns a new board with the given move applied.
             *
             * Asserts `isLegal(m)` in debug builds — callers on the movegen hot path
             * should ensure moves come from `getLegalMoves()` to avoid this overhead.
             * Castling is detected by a king moving exactly two files; the rook is
             * relocated accordingly. En passant is detected by a pawn capturing onto
             * the en passant square recorded in metadata; the captured pawn is removed
             * from the rank behind the destination. Castling rights are updated by
             * checking whether the from or to square matches any rook or king starting
             * square — this handles both rook moves and captures on those squares.
             * The halfmove clock is reset on pawn moves and captures, incremented
             * otherwise. The cache is fully invalidated on the returned board.
             */
            [[nodiscard]] inline Board push(Move m) const;

            // --- Move generation ---

            /**
             * Returns the list of fully legal moves from this position.
             *
             * Generates pseudo-legal moves via `getPseudoLegalMoves()`, then filters
             * them through `isLegalFast()`. Under double check, only king moves are
             * considered — this short-circuits the filter loop entirely since no other
             * piece can resolve a double check. The returned list is guaranteed to
             * contain only moves that leave the king out of check and satisfy all other
             * legality conditions. An empty list indicates checkmate or stalemate;
             * callers must distinguish between the two by checking `getCheckers()`.
             */
            [[nodiscard]] inline MoveList<256> getLegalMoves() const;

            /**
             * Returns a superset of legal moves from this position.
             *
             * Intentionally overgenerates to keep the implementation simple and
             * correct — the returned list may include moves that leave the king in
             * check, illegal slider moves through intervening pieces, and other
             * geometrically invalid moves. All such moves are filtered by
             * `isLegalFast()` inside `getLegalMoves()`. Sliders emit all squares
             * along unblocked rays from the movement table; path validity is not
             * checked here. Castling emits the king's destination square when the
             * path between king and rook is clear; check and pass-through square
             * validation is deferred to `isLegalFast()`.
             */
            [[nodiscard]] inline MoveList<512> getPseudoLegalMoves() const;

            /**
             * Returns true if the given move is legal in this position.
             *
             * The heavyweight legality check intended for UCI input validation.
             * First verifies the move exists in the pseudo-legal set, then delegates
             * to `isLegalFast()`. Do not use on the movegen hot path — prefer
             * `getLegalMoves()` which amortizes the pseudo-legal generation cost
             * across all moves.
             */
            [[nodiscard]] inline bool isLegal(Move m) const;

            /**
             * Returns true if the given move is legal in this position.
             *
             * The hot-path legality check used inside `getLegalMoves()`. Assumes the
             * move is already in the pseudo-legal set — passing an arbitrary move may
             * produce incorrect results. Operates directly on bitboards without calling
             * `push()`, avoiding infinite recursion through the debug assert in `push()`.
             *
             * Legality is determined by simulating the post-move board state in local
             * variables and checking whether the king is attacked. For sliders, alignment
             * and path clarity are verified against the movement and between tables. For
             * castling, the king's origin and pass-through squares are checked against
             * the pre- and post-move occupancy respectively. En passant correctly accounts
             * for the captured pawn's removal when evaluating king safety. The final king
             * safety check uses a post-move enemy bitboard with the captured piece removed.
             */
            [[nodiscard]] inline bool isLegalFast(Move m) const;

            // --- Metadata accessors ---

            /** Returns true if it is black's turn to move. */
            [[nodiscard]] inline bool isBlackTurn() const;

            /** Returns true if it is white's turn to move. */
            [[nodiscard]] inline bool isWhiteTurn() const;

            /** Returns true if white retains kingside castling rights. */
            [[nodiscard]] inline bool canWhiteKingsideCastle() const;

            /** Returns true if white retains queenside castling rights. */
            [[nodiscard]] inline bool canWhiteQueensideCastle() const;

            /** Returns true if black retains kingside castling rights. */
            [[nodiscard]] inline bool canBlackKingsideCastle() const;

            /** Returns true if black retains queenside castling rights. */
            [[nodiscard]] inline bool canBlackQueensideCastle() const;

            /** Returns true if an en passant capture is available on this turn. */
            [[nodiscard]] inline bool isEnPassantValid() const;

            /**
             * Returns the file index (0-7) of the en passant target square.
             * Only meaningful when `isEnPassantValid()` returns true.
             */
            [[nodiscard]] inline std::uint8_t getEnPassantFile() const;

            /** Returns the current halfmove clock value in [0, 100]. */
            [[nodiscard]] inline std::uint8_t getHalfMoveClock() const;

            // --- Cached derived values ---

            /**
             * Returns the combined occupancy of all pieces on the board.
             * Result is cached after the first call and invalidated by `push()`.
             */
            [[nodiscard]] inline uint64_t getOccupancy() const;

            /**
             * Returns the occupancy of all white pieces.
             * Result is cached after the first call and invalidated by `push()`.
             */
            [[nodiscard]] inline uint64_t getWhiteOccupancy() const;

            /**
             * Returns the occupancy of all black pieces.
             * Result is cached after the first call and invalidated by `push()`.
             */
            [[nodiscard]] inline uint64_t getBlackOccupancy() const;

            /**
             * Returns the current white bitboards.
             */
            [[nodiscard]] inline std::array<uint64_t, 6> getWhiteBitboards() const {
                return whiteBitboards;
            }

            /**
             * Returns the current black bitboards.
             */
            [[nodiscard]] inline std::array<uint64_t, 6> getBlackBitboards() const {
                return blackBitboards;
            }

            /**
             * Returns a bitboard of where the current pieces of the opponent's are that are
             * checking the side to move's king.
             */
            [[nodiscard]] inline uint64_t getCheckers() const;

            /**
             * Returns the current zobrist hash of the position.
             */
            [[nodiscard]] inline uint64_t getHash() const;

        private:
            std::array<uint64_t, 6> whiteBitboards;  ///< One bitboard per `PieceType` for white,
                                                     ///< indexed by `PieceType` value.
            std::array<uint64_t, 6> blackBitboards;  ///< One bitboard per `PieceType` for black,
                                                     ///< indexed by `PieceType` value.

            uint64_t _zobristHash;  ///< Cached Zobrist hash of this position.
            mutable uint64_t _checkers;     ///< Cached bitboard for the checkers of this position.

            mutable uint64_t metadata;  ///< Packed position metadata; see `BoardMetadataFlags`.
            uint64_t __padding;         ///< Unused padding, DO NOT SET.
    };

    static_assert(sizeof(Board) == 128);

}  // namespace aleph::chess

#include "board.inl"