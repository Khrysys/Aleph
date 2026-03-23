#pragma once

#include <iostream>

#include <aleph/platform.hpp>

#include "../attack_tables.hpp" 
#include "../board.hpp"

namespace aleph::chess {
    namespace detail {
        [[nodiscard]] inline bool isAttackedBy(uint8_t sqIdx, uint64_t occ,
                                               const std::array<uint64_t, 6>& attackers,
                                               bool attackersAreBlack) {
            uint64_t sqBit = 1ULL << sqIdx;

            // Pawns
            uint64_t pawns = attackers[PAWN];
            while (pawns) {
                uint8_t sq  = static_cast<uint8_t>(platform::tzcnt(pawns));
                pawns      &= pawns - 1;
                if (attackTables.movement[attackersAreBlack ? PAWN + 6 : PAWN][sq] & sqBit)
                    return true;
            }

            // Knights
            if (attackTables.movement[KNIGHT][sqIdx] & attackers[KNIGHT]) return true;

            // King
            if (attackTables.movement[KING][sqIdx] & attackers[KING]) return true;

            // Diagonal sliders
            uint64_t sliders = attackers[BISHOP] | attackers[QUEEN];
            while (sliders) {
                uint8_t sq  = static_cast<uint8_t>(platform::tzcnt(sliders));
                sliders    &= sliders - 1;
                if (attackTables.movement[BISHOP][sq] & sqBit)
                    if ((attackTables.between[sq][sqIdx] & occ) == 0) return true;
            }

            // Orthogonal sliders
            sliders = attackers[ROOK] | attackers[QUEEN];
            while (sliders) {
                uint8_t sq  = static_cast<uint8_t>(platform::tzcnt(sliders));
                sliders    &= sliders - 1;
                if (attackTables.movement[ROOK][sq] & sqBit)
                    if ((attackTables.between[sq][sqIdx] & occ) == 0) return true;
            }

            return false;
        }
    }  // namespace detail

    MoveList<256> Board::getLegalMoves() const {
        auto pseudoLegal = getPseudoLegalMoves();
        MoveList<256> result{};

        if (platform::popcnt(getCheckers()) == 2) {
            // Double check — only king moves can be legal
            for (const auto& m : pseudoLegal) {
                if (get(m.from()).type() == KING && isLegalFast(m)) result += m;
            }
            return result;
        }

        for (const auto& m : pseudoLegal) {
            if (isLegalFast(m)) {
                result += m;
            }
        }
        return result;
    }

    bool Board::isLegal(Move m) const {
        return getPseudoLegalMoves().contains(m) && isLegalFast(m);
    }

    bool Board::isLegalFast(Move m) const {
        Square from                = m.from();
        Square to                  = m.to();

        uint8_t fromIdx            = static_cast<uint8_t>(from);
        uint8_t toIdx              = static_cast<uint8_t>(to);
        uint64_t fromBit           = 1ULL << fromIdx;
        uint64_t toBit             = 1ULL << toIdx;

        bool blackTurn             = isBlackTurn();

        const auto& ownBitboards   = blackTurn ? blackBitboards : whiteBitboards;
        const auto& enemyBitboards = blackTurn ? whiteBitboards : blackBitboards;

        uint64_t ownOcc            = blackTurn ? getBlackOccupancy() : getWhiteOccupancy();
        uint64_t enemyOcc          = blackTurn ? getWhiteOccupancy() : getBlackOccupancy();
        uint64_t occ               = getOccupancy();

        // --- Not capturing own piece ---
        if (ownOcc & toBit) return false;

        // --- Determine moving piece type ---
        PieceType movingPiece = get(from).type();
        if (movingPiece == NONE) return false;

        // --- Slider path check ---
        if (movingPiece == BISHOP || movingPiece == ROOK || movingPiece == QUEEN) {
            bool aligned = false;
            if (movingPiece == BISHOP || movingPiece == QUEEN)
                aligned |= (bool)(attackTables.movement[BISHOP][fromIdx] & toBit);
            if (movingPiece == ROOK || movingPiece == QUEEN)
                aligned |= (bool)(attackTables.movement[ROOK][fromIdx] & toBit);
            if (!aligned) return false;
            if (attackTables.between[fromIdx][toIdx] & occ) return false;
        }

        // --- Compute post-move occupancy ---
        uint64_t newOwnOcc     = (ownOcc & ~fromBit) | toBit;
        uint64_t newEnemyOcc   = enemyOcc & ~toBit;

        // --- En passant --- remove captured pawn from enemy occupancy
        uint64_t epCapturedBit = 0;
        if (movingPiece == PAWN && isEnPassantValid()) {
            uint8_t epFile = getEnPassantFile();
            if (to.file() == epFile && from.file() != epFile) {
                uint8_t capturedRank = blackTurn ? static_cast<uint8_t>(to.rank() + 1)
                                                 : static_cast<uint8_t>(to.rank() - 1);
                Square capturedSq(capturedRank, epFile);
                epCapturedBit  = 1ULL << static_cast<uint8_t>(capturedSq);
                newEnemyOcc   &= ~epCapturedBit;
            }
        }

        uint64_t newOcc = newOwnOcc | newEnemyOcc;

        // --- Find king square after move ---
        uint64_t kingBB = ownBitboards[KING];
        if (movingPiece == KING) kingBB = toBit;
        uint8_t kingSqIdx = static_cast<uint8_t>(platform::tzcnt(kingBB));

        // --- Castling: king must not pass through or land on attacked square ---
        if (movingPiece == KING) {
            int8_t fileDelta = static_cast<int8_t>(to.file()) - static_cast<int8_t>(from.file());
            if (fileDelta == 2 || fileDelta == -2) {
                uint8_t passingFile  = static_cast<uint8_t>(from.file() + (fileDelta > 0 ? 1 : -1));
                uint8_t passingSqIdx = static_cast<uint8_t>(Square(from.rank(), passingFile));
                if (detail::isAttackedBy(fromIdx, occ, enemyBitboards, !blackTurn)) return false;
                if (detail::isAttackedBy(passingSqIdx, newOcc, enemyBitboards, !blackTurn))
                    return false;
            }
        }

        // --- Post-move enemy bitboards ---
        std::array<uint64_t, 6> enemyBB;
        for (int i = 0; i < 6; i++) enemyBB[i] = enemyBitboards[i] & ~toBit & ~epCapturedBit;

        // --- Check if king is attacked after move ---
        if (detail::isAttackedBy(kingSqIdx, newOcc, enemyBB, !blackTurn)) return false;

        return true;
    }

    MoveList<512> Board::getPseudoLegalMoves() const {
        MoveList<512> result{};

        bool blackTurn             = isBlackTurn();

        const auto& ownBitboards   = blackTurn ? blackBitboards : whiteBitboards;
        const auto& enemyBitboards = blackTurn ? whiteBitboards : blackBitboards;

        uint64_t ownOcc            = blackTurn ? getBlackOccupancy() : getWhiteOccupancy();
        uint64_t enemyOcc          = blackTurn ? getWhiteOccupancy() : getBlackOccupancy();
        uint64_t occ               = getOccupancy();

        // Promotion ranks
        uint8_t promotionRank      = blackTurn ? 0 : 7;
        uint8_t startingRank       = blackTurn ? 6 : 1;

        // Direction of pawn push
        int8_t pushDir             = blackTurn ? -1 : 1;

        // Attack table index offset for color
        int colorOffset            = blackTurn ? 6 : 0;

        for (int pieceIdx = 0; pieceIdx < 6; pieceIdx++) {
            uint64_t bb = ownBitboards[pieceIdx];

            while (bb) {
                uint8_t fromIdx  = static_cast<uint8_t>(aleph::platform::tzcnt(bb));
                bb              &= bb - 1;
                Square from(fromIdx);

                if (pieceIdx == PAWN) {
                    // --- Single push ---
                    uint8_t pushRank = static_cast<uint8_t>(from.rank() + pushDir);
                    Square pushSq(pushRank, from.file());
                    uint64_t pushBit = 1ULL << static_cast<uint8_t>(pushSq);

                    if (!(occ & pushBit)) {
                        if (pushRank == promotionRank) {
                            result += Move(from, pushSq, QUEEN);
                            result += Move(from, pushSq, ROOK);
                            result += Move(from, pushSq, BISHOP);
                            result += Move(from, pushSq, KNIGHT);
                        } else {
                            result += Move(from, pushSq);

                            // --- Double push ---
                            if (from.rank() == startingRank) {
                                uint8_t doublePushRank =
                                    static_cast<uint8_t>(from.rank() + pushDir * 2);
                                Square doublePushSq(doublePushRank, from.file());
                                uint64_t doublePushBit = 1ULL << static_cast<uint8_t>(doublePushSq);
                                if (!(occ & doublePushBit)) result += Move(from, doublePushSq);
                            }
                        }
                    }

                    // --- Pawn captures ---
                    uint64_t captureMask = attackTables.movement[colorOffset + PAWN][fromIdx];
                    uint64_t captures    = captureMask & enemyOcc;
                    while (captures) {
                        uint8_t toIdx  = static_cast<uint8_t>(std::countr_zero(captures));
                        captures      &= captures - 1;
                        Square to(toIdx);

                        if (to.rank() == promotionRank) {
                            result += Move(from, to, QUEEN);
                            result += Move(from, to, ROOK);
                            result += Move(from, to, BISHOP);
                            result += Move(from, to, KNIGHT);
                        } else {
                            result += Move(from, to);
                        }
                    }

                    // --- En passant ---
                    if (isEnPassantValid()) {
                        uint8_t epFile = getEnPassantFile();
                        uint8_t epRank = blackTurn ? 2 : 5;
                        // Pawn must be on adjacent file and correct rank to capture
                        if (from.rank() == static_cast<uint8_t>(epRank - pushDir) &&
                            (from.file() == epFile - 1 || from.file() == epFile + 1)) {
                            Square epSq(epRank, epFile);
                            result += Move(from, epSq);
                        }
                    }

                } else if (pieceIdx == KING) {
                    // --- Normal king moves ---
                    uint64_t moves = attackTables.movement[colorOffset + KING][fromIdx] & ~ownOcc;
                    while (moves) {
                        uint8_t toIdx  = static_cast<uint8_t>(std::countr_zero(moves));
                        moves         &= moves - 1;
                        result        += Move(from, Square(toIdx));
                    }

                    // --- Castling ---
                    if (!blackTurn) {
                        if (canWhiteKingsideCastle()) {
                            constexpr uint64_t WK_PATH = (1ULL << 5) | (1ULL << 6);
                            if (!(occ & WK_PATH)) result += Move(from, Square(0, 6));
                        }
                        if (canWhiteQueensideCastle()) {
                            constexpr uint64_t WQ_PATH = (1ULL << 1) | (1ULL << 2) | (1ULL << 3);
                            if (!(occ & WQ_PATH)) result += Move(from, Square(0, 2));
                        }
                    } else {
                        if (canBlackKingsideCastle()) {
                            constexpr uint64_t BK_PATH = (1ULL << 61) | (1ULL << 62);
                            if (!(occ & BK_PATH)) result += Move(from, Square(7, 6));
                        }
                        if (canBlackQueensideCastle()) {
                            constexpr uint64_t BQ_PATH = (1ULL << 57) | (1ULL << 58) | (1ULL << 59);
                            if (!(occ & BQ_PATH)) result += Move(from, Square(7, 2));
                        }
                    }

                } else {
                    // --- Non-pawn, non-king pieces (knight, bishop, rook, queen) ---
                    uint64_t moves =
                        attackTables.movement[colorOffset + pieceIdx][fromIdx] & ~ownOcc;
                    while (moves) {
                        uint8_t toIdx  = static_cast<uint8_t>(std::countr_zero(moves));
                        moves         &= moves - 1;
                        result        += Move(from, Square(toIdx));
                    }
                }
            }
        }

        return result;
    }
}  // namespace aleph::chess