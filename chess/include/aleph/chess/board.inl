#pragma once

#include "board.hpp"

namespace aleph::chess {
    std::uint64_t Board::blackOccupancy() const {
        if (_cacheValid & BLACK_OCCUPANCY_VALID) {
            return _blackOccupancy;
        }
        std::uint64_t r = 0;
        for (auto i = 0; i < blackBitboards.size(); i++) r |= blackBitboards[i];
        _blackOccupancy  = r;
        _cacheValid     |= BLACK_OCCUPANCY_VALID;
        return r;
    }

    std::uint64_t Board::whiteOccupancy() const {
        if (_cacheValid & WHITE_OCCUPANCY_VALID) {
            return _whiteOccupancy;
        }
        std::uint64_t r = 0;
        for (auto i = 0; i < whiteBitboards.size(); i++) r |= whiteBitboards[i];
        _whiteOccupancy  = r;
        _cacheValid     |= WHITE_OCCUPANCY_VALID;
        return r;
    }

    std::uint64_t Board::occupancy() const {
        if (_cacheValid & OCCUPANCY_VALID) {
            return _occupancy;
        }
        std::uint64_t r   = blackOccupancy() | whiteOccupancy();
        _occupancy   = r;
        _cacheValid |= OCCUPANCY_VALID;
        return r;
    }

    bool Board::isBlackTurn() const { return metadata & BoardMetadataFlags::BLACK_TO_MOVE; }

    Board Board::push(Move m) const {
#ifndef NDEBUG
        assert(isLegal(m));
#endif
        Board nextBoard = *this;

        // TODO: Optimize this, there's certainly a way to keep it from needing to be redone every
        // position Invalidate cache
        return nextBoard;
    }

    MoveList<256> Board::getLegalMoves() const {
        auto moves = getPseudoLegalMoves();
        MoveList<256> result{};

        for (Move m : moves)
            if (isLegalFast(m)) result += m;

        return result;
    }

    MoveList<512> Board::getPseudoLegalMoves() const {
        MoveList<512> result{};
        if (isBlackTurn()) {
            for (std::size_t i = 0; i < blackBitboards.size(); i++) {
                auto bb = blackBitboards[i];
                while (bb) {
                    Square from = std::countr_zero(bb);
                    auto moves  = attackTables.movement[i + 6][from];
                    while (moves) {
                        Square to  = std::countr_zero(moves);
                        auto move  = Move(from, to);
                        result    += move;
                        moves     ^= (1ULL << to);
                    }
                    bb ^= (1ULL << from);
                }
            }
        } else {
            for (size_t i = 0; i < whiteBitboards.size(); i++) {
                auto bb = whiteBitboards[i];
                while (bb) {
                    Square from = std::countr_zero(bb);
                    auto moves  = attackTables.movement[i][from];
                    while (moves) {
                        Square to  = std::countr_zero(moves);
                        auto move  = Move(from, to);
                        result    += move;
                        moves     &= moves - 1;
                    }
                    bb &= bb - 1;
                }
            }
        }
        return result;
    }
}  // namespace aleph::chess