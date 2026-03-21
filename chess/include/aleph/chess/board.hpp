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
        constexpr std::string_view STARTING_POSITION_FEN =
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    }

    /**
     * Encodes the fields packed into the 32-bit `metadata` word of `Board`.
     *
     * Layout:
     *   [2:0]   EN_PASSANT_FILE_MASK  — file index (0-7) of the en passant target square.
     *   [3]     EN_PASSANT_VALID      — set if an en passant capture is currently available.
     *   [4]     BLACK_TO_MOVE         — set if it is black's turn to move.
     *   [5]     WHITE_KINGSIDE_CASTLE — set if white retains kingside castling rights.
     *   [6]     WHITE_QUEENSIDE_CASTLE— set if white retains queenside castling rights.
     *   [7]     BLACK_KINGSIDE_CASTLE — set if black retains kingside castling rights.
     *   [8]     BLACK_QUEENSIDE_CASTLE— set if black retains queenside castling rights.
     *   [15:9]  HALF_MOVE_CLOCK        — halfmove clock for the 50-move rule (0-100).
     *                                   Extract via `(metadata & HALF_MOVE_CLOCK) >> 9`.
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
        HALF_MOVE_CLOCK         = 0x0000FE00
    };

    /**
     * Bitmask flags indicating which lazy cache fields on `Board` are currently valid.
     *
     * Cache fields are invalidated by zeroing the relevant bits in `_cacheValid`
     * whenever `push()` produces a new board. Fields are recomputed on demand and
     * marked valid at that point. All fields start invalid on a freshly constructed board.
     */
    enum BoardCacheValidFlags : uint8_t {
        OCCUPANCY_VALID       = 0x01,  ///< `_occupancy` is up to date.
        WHITE_OCCUPANCY_VALID = 0x02,  ///< `_whiteOccupancy` is up to date.
        BLACK_OCCUPANCY_VALID = 0x04,  ///< `_blackOccupancy` is up to date.
        ZOBRIST_HASH_VALID    = 0x08   ///< `_zobristHash` is up to date.
    };

    /**
     * Represents a chess position as an immutable value type.
     *
     * The board state is encoded as twelve 64-bit bitboards (six per side, one per
     * piece type), a 32-bit metadata word encoding side-to-move, castling rights,
     * en passant state, and the halfmove clock. See `BoardMetadataFlags` for the
     * exact bit layout.
     *
     * Mutating the position is done exclusively via `push()`, which returns a new
     * `Board` with the move applied. The original board is never modified. Mutable
     * fields (`_cacheValid`, `_occupancy`, etc.) exist only for lazy recomputation
     * of derived values and do not affect observable board state.
     *
     * Castling and en passant are detected contextually in `push()` rather than
     * encoded in move bits — a king moving two squares triggers castling, and a pawn
     * capturing onto the en passant square triggers en passant removal of the captured
     * pawn.
     *
     * In debug builds, `push()` asserts `isLegal(m)` before applying the move.
     * `isLegalFast()` is the hot-path legality check used inside `getLegalMoves()`;
     * it must never call `push()` to avoid infinite recursion through the debug assert.
     */
    class Board {
        public:
            inline Board() : Board(detail::STARTING_POSITION_FEN) {}
            inline Board(std::string_view fen);

            /**
             * 
             */
            [[nodiscard]] inline Piece get(Square sq) const;

            /**
             * Returns a new board with the given move applied.
             * Asserts `isLegal(m)` in debug builds. Detects castling and en passant
             * contextually from the move and current board state.
             */
            [[nodiscard]] inline Board push(Move m) const;

            /** Returns the list of fully legal moves from this position. */
            [[nodiscard]] inline MoveList<256> getLegalMoves() const;

            /**
             * Returns a superset of legal moves from this position.
             * The list may include illegal moves (e.g. leaving the king in check);
             * all entries are filtered by `isLegalFast()` in `getLegalMoves()`.
             */
            [[nodiscard]] inline MoveList<512> getPseudoLegalMoves() const;

            /**
             * Returns true if the given move is legal in this position.
             * This is the heavyweight check used for UCI input validation — it
             * first checks pseudo-legality and then delegates to `isLegalFast()`.
             * Do not use on the movegen hot path.
             */
            [[nodiscard]] inline bool isLegal(Move m) const;

            /**
             * Returns true if the given move is legal in this position.
             * This is the hot-path check used inside `getLegalMoves()`. It operates
             * directly on bitboards without calling `push()`, avoiding infinite
             * recursion through the debug assert in `push()`.
             */
            [[nodiscard]] inline bool isLegalFast(Move m) const;

            /**
             * Returns the combined occupancy of all pieces on the board.
             * Result is cached after the first call and invalidated by `push()`.
             */
            [[nodiscard]] inline uint64_t occupancy() const;

            /**
             * Returns the occupancy of all white pieces.
             * Result is cached after the first call and invalidated by `push()`.
             */
            [[nodiscard]] inline uint64_t whiteOccupancy() const;

            /**
             * Returns the occupancy of all black pieces.
             * Result is cached after the first call and invalidated by `push()`.
             */
            [[nodiscard]] inline uint64_t blackOccupancy() const;

            // Metadata accessors

            /**
             * Returns true if it is black's turn to move.
             * Reads directly from the `BLACK_TO_MOVE` flag in `metadata`.
             */
            [[nodiscard]] inline bool isBlackTurn() const;

            /**
             * Returns true if it is white's turn to move.
             * Reads directly from the `BLACK_TO_MOVE` flag in `metadata`.
             */
            [[nodiscard]] inline bool isWhiteTurn() const;

            /**
             *
             */
            [[nodiscard]] inline bool canWhiteKingsideCastle() const;

            [[nodiscard]] inline bool canWhiteQueensideCastle() const;

            [[nodiscard]] inline bool canBlackKingsideCastle() const;

            [[nodiscard]] inline bool canBlackQueensideCastle() const;

            [[nodiscard]] inline bool isEnPassantValid() const;

            [[nodiscard]] inline std::uint8_t enPassantFile() const;

            [[nodiscard]] inline std::uint8_t halfMoveClock() const;

        private:
            std::array<uint64_t, 6> whiteBitboards;  ///< One bitboard per PieceType for white.
            std::array<uint64_t, 6> blackBitboards;  ///< One bitboard per PieceType for black.
            uint32_t metadata;  ///< Packed position metadata; see BoardMetadataFlags.

            mutable uint8_t
                _cacheValid;  ///< Bitmask of valid cache fields; see BoardCacheValidFlags.
            mutable uint64_t _occupancy;       ///< Cached combined occupancy.
            mutable uint64_t _whiteOccupancy;  ///< Cached white occupancy.
            mutable uint64_t _blackOccupancy;  ///< Cached black occupancy.
            mutable uint64_t _zobristHash;     ///< Cached Zobrist hash of this position.
    };

}  // namespace aleph::chess

#include "board.inl"