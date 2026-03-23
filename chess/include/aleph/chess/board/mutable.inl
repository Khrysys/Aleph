#pragma once

#include <functional>
#include <numeric>

#include "../attack_tables.hpp"
#include "../board.hpp"

namespace aleph::chess {
    std::uint64_t Board::getOccupancy() const {
        if ((_cacheValid & OCCUPANCY_VALID) == 0) {
            _occupancy   = getWhiteOccupancy() | getBlackOccupancy();
            _cacheValid |= OCCUPANCY_VALID;
        }
        return _occupancy;
    }

    std::uint64_t Board::getWhiteOccupancy() const {
        if ((_cacheValid & WHITE_OCCUPANCY_VALID) == 0) {
            _whiteOccupancy  = std::reduce(whiteBitboards.begin(), whiteBitboards.end(), 0ULL,
                                           std::bit_or<uint64_t>{});
            _cacheValid     |= WHITE_OCCUPANCY_VALID;
        }
        return _whiteOccupancy;
    }

    std::uint64_t Board::getBlackOccupancy() const {
        if ((_cacheValid & BLACK_OCCUPANCY_VALID) == 0) {
            _blackOccupancy  = std::reduce(blackBitboards.begin(), blackBitboards.end(), 0ULL,
                                           std::bit_or<uint64_t>{});
            _cacheValid     |= BLACK_OCCUPANCY_VALID;
        }
        return _blackOccupancy;
    }

    uint64_t Board::getCheckers() const {
        if ((_cacheValid & CHECKERS_VALID) == 0) {
            bool blackTurn             = isBlackTurn();

            const auto& ownBitboards   = blackTurn ? blackBitboards : whiteBitboards;
            const auto& enemyBitboards = blackTurn ? whiteBitboards : blackBitboards;

            uint8_t kingSq             = static_cast<uint8_t>(platform::tzcnt(ownBitboards[KING]));
            uint64_t kingSqBit         = 1ULL << kingSq;
            uint64_t occ               = getOccupancy();

            _checkers                  = 0;

            // Pawn checkers
            uint64_t pawns             = enemyBitboards[PAWN];
            while (pawns) {
                uint8_t sq  = static_cast<uint8_t>(platform::tzcnt(pawns));
                pawns      &= pawns - 1;
                if (attackTables.movement[blackTurn ? PAWN : (PAWN + 6)][sq] & kingSqBit)
                    _checkers |= 1ULL << sq;
            }

            // Knight checkers
            uint64_t knights = enemyBitboards[KNIGHT];
            while (knights) {
                uint8_t sq  = static_cast<uint8_t>(platform::tzcnt(knights));
                knights    &= knights - 1;
                if (attackTables.movement[KNIGHT][sq] & kingSqBit) _checkers |= 1ULL << sq;
            }

            // Bishop and diagonal queen checkers
            uint64_t sliders = enemyBitboards[BISHOP] | enemyBitboards[QUEEN];
            while (sliders) {
                uint8_t sq  = static_cast<uint8_t>(platform::tzcnt(sliders));
                sliders    &= sliders - 1;
                if (attackTables.movement[BISHOP][sq] & kingSqBit)
                    if ((attackTables.between[sq][kingSq] & occ) == 0) _checkers |= 1ULL << sq;
            }

            // Rook and orthogonal queen checkers
            sliders = enemyBitboards[ROOK] | enemyBitboards[QUEEN];
            while (sliders) {
                uint8_t sq  = static_cast<uint8_t>(platform::tzcnt(sliders));
                sliders    &= sliders - 1;
                if (attackTables.movement[ROOK][sq] & kingSqBit)
                    if ((attackTables.between[sq][kingSq] & occ) == 0) _checkers |= 1ULL << sq;
            }

            _cacheValid |= CHECKERS_VALID;
        }
        return _checkers;
    }

}  // namespace aleph::chess