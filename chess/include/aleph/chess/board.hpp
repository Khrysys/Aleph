#pragma once

#include <array>
#include <cassert>
#include <cstdint>

#include <aleph/platform.hpp>

#include "move.hpp"
#include "move_list.hpp"

namespace aleph::chess {
    enum BoardMetadataFlags : std::uint32_t {
        EN_PASSANT_FILE_MASK   = 0x00000007,
        EN_PASSANT_VALID       = 0x00000008,
        BLACK_TO_MOVE          = 0x00000010,
        WHITE_KINGSIDE_CASTLE  = 0x00000020,
        WHITE_QUEENSIDE_CASTLE = 0x00000040,
        BLACK_KINGSIDE_CASTLE  = 0x00000080,
        BLACK_QUEENSIDE_CASTLE = 0x00000100,
        HALFMOVE_CLOCK         = 0x0000FE00
    };

    enum BoardCacheValidFlags : std::uint8_t {
        OCCUPANCY_VALID       = 0x01,
        WHITE_OCCUPANCY_VALID = 0x02,
        BLACK_OCCUPANCY_VALID = 0x04,
        ZOBRIST_HASH_VALID    = 0x08
    };

    class Board {
        public:
            [[nodiscard]] inline Board push(Move m) const;
            [[nodiscard]] inline MoveList<256> getLegalMoves() const;
            [[nodiscard]] inline MoveList<512> getPseudoLegalMoves() const;
            [[nodiscard]] inline bool isLegal(Move m) const;
            [[nodiscard]] inline bool isLegalFast(Move m) const;

            [[nodiscard]] inline uint64_t occupancy() const;
            [[nodiscard]] inline uint64_t whiteOccupancy() const;
            [[nodiscard]] inline uint64_t blackOccupancy() const;

            [[nodiscard]] inline bool isBlackTurn() const;

        private:
            std::array<uint64_t, 6> whiteBitboards;
            std::array<uint64_t, 6> blackBitboards;
            uint32_t metadata;

            mutable uint8_t _cacheValid;
            mutable uint64_t _occupancy;
            mutable uint64_t _whiteOccupancy;
            mutable uint64_t _blackOccupancy;
            mutable uint64_t _zobristHash;
    };
}  // namespace aleph::chess

#ifndef ALEPH_NO_IMPLEMENTATION
    #include "board.inl"
#endif