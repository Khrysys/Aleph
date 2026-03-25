/**
 * @file include/aleph/chess/board/fen.inl
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include "../board.hpp"
#include "../zobrist.hpp"

namespace aleph::chess {
    Board::Board(std::string_view fen)
        : whiteBitboards{}, blackBitboards{}, metadata(0), _zobristHash(0), _checkers(0) {
        // --- Split FEN into fields ---
        std::array<std::string_view, 6> fields;
        std::size_t fieldCount = 0;
        std::size_t start      = 0;

        for (std::size_t i = 0; i <= fen.size(); ++i) {
            if (i == fen.size() || fen[i] == ' ') {
                if (fieldCount >= 6) throw std::invalid_argument("FEN has too many fields");
                fields[fieldCount++] = fen.substr(start, i - start);
                start                = i + 1;
            }
        }
        if (fieldCount < 4)
            throw std::invalid_argument("FEN has too few fields (minimum 4 required)");

        // --- Field 1: Piece placement ---
        {
            std::size_t rankIdx = 7;  // FEN starts from rank 8 (index 7)
            std::size_t fileIdx = 0;

            for (char c : fields[0]) {
                if (c == '/') {
                    if (fileIdx != 8)
                        throw std::invalid_argument("FEN rank does not sum to 8 squares");
                    if (rankIdx == 0) throw std::invalid_argument("FEN has too many ranks");
                    --rankIdx;
                    fileIdx = 0;
                } else if (c >= '1' && c <= '8') {
                    fileIdx += static_cast<std::size_t>(c - '0');
                    if (fileIdx > 8) throw std::invalid_argument("FEN rank exceeds 8 squares");
                } else {
                    if (detail::PIECE_TYPE_CHARS.find(c) == std::string_view::npos)
                        throw std::invalid_argument(
                            std::string("FEN contains invalid piece character: ") + c);
                    if (fileIdx >= 8) throw std::invalid_argument("FEN rank exceeds 8 squares");

                    Piece p(c);
                    Square sq(static_cast<uint8_t>(rankIdx), static_cast<uint8_t>(fileIdx));
                    uint64_t bit = 1ULL << static_cast<uint8_t>(sq);

                    if (p.isBlack())
                        blackBitboards[p.type()] |= bit;
                    else
                        whiteBitboards[p.type()] |= bit;

                    ++fileIdx;
                }
            }

            if (fileIdx != 8)
                throw std::invalid_argument("FEN final rank does not sum to 8 squares");
            if (rankIdx != 0) throw std::invalid_argument("FEN has too few ranks");
        }

        // --- Piece validity checks ---
        {
            if (platform::popcnt(whiteBitboards[KING]) != 1)
                throw std::invalid_argument("FEN must have exactly one white king");
            if (platform::popcnt(blackBitboards[KING]) != 1)
                throw std::invalid_argument("FEN must have exactly one black king");

            if (platform::popcnt(whiteBitboards[PAWN]) > 8)
                throw std::invalid_argument("FEN has too many white pawns");
            if (platform::popcnt(blackBitboards[PAWN]) > 8)
                throw std::invalid_argument("FEN has too many black pawns");

            constexpr uint64_t RANK_1 = 0x00000000000000FFULL;
            constexpr uint64_t RANK_8 = 0xFF00000000000000ULL;
            if (whiteBitboards[PAWN] & (RANK_1 | RANK_8))
                throw std::invalid_argument("FEN has pawns on back rank");
            if (blackBitboards[PAWN] & (RANK_1 | RANK_8))
                throw std::invalid_argument("FEN has pawns on back rank");

            uint32_t whitePieces = 0;
            uint32_t blackPieces = 0;
            for (int i = 0; i < 6; i++) {
                whitePieces += platform::popcnt(whiteBitboards[i]);
                blackPieces += platform::popcnt(blackBitboards[i]);
            }
            if (whitePieces > 16) throw std::invalid_argument("FEN has too many white pieces");
            if (blackPieces > 16) throw std::invalid_argument("FEN has too many black pieces");

            uint64_t allWhite = 0, allBlack = 0;
            for (int i = 0; i < 6; i++) {
                if (whiteBitboards[i] & allWhite)
                    throw std::invalid_argument("FEN has overlapping white pieces");
                if (blackBitboards[i] & allBlack)
                    throw std::invalid_argument("FEN has overlapping black pieces");
                allWhite |= whiteBitboards[i];
                allBlack |= blackBitboards[i];
            }
            if (allWhite & allBlack)
                throw std::invalid_argument("FEN has overlapping white and black pieces");
        }

        // --- Field 2: Side to move ---
        {
            if (fields[1] == "b")
                metadata |= BLACK_TO_MOVE;
            else if (fields[1] != "w")
                throw std::invalid_argument("FEN side to move must be 'w' or 'b'");
        }

        // --- Field 3: Castling rights ---
        {
            constexpr uint64_t WHITE_KING_START = 1ULL << static_cast<uint8_t>(Square(0, 4));
            constexpr uint64_t WHITE_KS_ROOK    = 1ULL << static_cast<uint8_t>(Square(0, 7));
            constexpr uint64_t WHITE_QS_ROOK    = 1ULL << static_cast<uint8_t>(Square(0, 0));
            constexpr uint64_t BLACK_KING_START = 1ULL << static_cast<uint8_t>(Square(7, 4));
            constexpr uint64_t BLACK_KS_ROOK    = 1ULL << static_cast<uint8_t>(Square(7, 7));
            constexpr uint64_t BLACK_QS_ROOK    = 1ULL << static_cast<uint8_t>(Square(7, 0));

            if (fields[2] != "-") {
                for (char c : fields[2]) {
                    switch (c) {
                        case 'K':
                            if (!(whiteBitboards[KING] & WHITE_KING_START))
                                throw std::invalid_argument(
                                    "FEN claims white kingside castling but king not on e1");
                            if (!(whiteBitboards[ROOK] & WHITE_KS_ROOK))
                                throw std::invalid_argument(
                                    "FEN claims white kingside castling but rook not on h1");
                            metadata |= WHITE_KINGSIDE_CASTLE;
                            break;
                        case 'Q':
                            if (!(whiteBitboards[KING] & WHITE_KING_START))
                                throw std::invalid_argument(
                                    "FEN claims white queenside castling but king not on e1");
                            if (!(whiteBitboards[ROOK] & WHITE_QS_ROOK))
                                throw std::invalid_argument(
                                    "FEN claims white queenside castling but rook not on a1");
                            metadata |= WHITE_QUEENSIDE_CASTLE;
                            break;
                        case 'k':
                            if (!(blackBitboards[KING] & BLACK_KING_START))
                                throw std::invalid_argument(
                                    "FEN claims black kingside castling but king not on e8");
                            if (!(blackBitboards[ROOK] & BLACK_KS_ROOK))
                                throw std::invalid_argument(
                                    "FEN claims black kingside castling but rook not on h8");
                            metadata |= BLACK_KINGSIDE_CASTLE;
                            break;
                        case 'q':
                            if (!(blackBitboards[KING] & BLACK_KING_START))
                                throw std::invalid_argument(
                                    "FEN claims black queenside castling but king not on e8");
                            if (!(blackBitboards[ROOK] & BLACK_QS_ROOK))
                                throw std::invalid_argument(
                                    "FEN claims black queenside castling but rook not on a8");
                            metadata |= BLACK_QUEENSIDE_CASTLE;
                            break;
                        default:
                            throw std::invalid_argument(
                                std::string("FEN castling field contains invalid character: ") + c);
                    }
                }
            }
        }

        // --- Field 4: En passant ---
        {
            if (fields[3] != "-") {
                if (fields[3].size() != 2)
                    throw std::invalid_argument("FEN en passant field must be a square or '-'");

                uint8_t file = static_cast<uint8_t>(fields[3][0] - 'a');
                uint8_t rank = static_cast<uint8_t>(fields[3][1] - '1');

                if (file > 7) throw std::invalid_argument("FEN en passant file is invalid");
                if (rank > 7) throw std::invalid_argument("FEN en passant rank is invalid");

                // En passant rank must be 2 (black to move, white just pushed) or
                // 5 (white to move, black just pushed) — 0-indexed.
                if (isBlackTurn() && rank != 2)
                    throw std::invalid_argument(
                        "FEN en passant rank must be 3 when black is to move");
                if (isWhiteTurn() && rank != 5)
                    throw std::invalid_argument(
                        "FEN en passant rank must be 6 when white is to move");

                // There must be a pawn on the rank it landed on after the double push.
                uint64_t pawnBit = isBlackTurn() ? 1ULL << static_cast<uint8_t>(Square(3, file))
                                                 : 1ULL << static_cast<uint8_t>(Square(4, file));

                if (isBlackTurn() && !(whiteBitboards[PAWN] & pawnBit))
                    throw std::invalid_argument(
                        "FEN en passant square is invalid: no white pawn on expected square");
                if (isWhiteTurn() && !(blackBitboards[PAWN] & pawnBit))
                    throw std::invalid_argument(
                        "FEN en passant square is invalid: no black pawn on expected square");

                metadata |= EN_PASSANT_VALID;
                metadata |= (file & EN_PASSANT_FILE_MASK);
            }
        }

        // --- Field 5: Halfmove clock (optional) ---
        if (fieldCount >= 5) {
            uint32_t halfmove = 0;
            for (char c : fields[4]) {
                if (c < '0' || c > '9')
                    throw std::invalid_argument("FEN halfmove clock is not a valid integer");
                halfmove = halfmove * 10 + static_cast<uint32_t>(c - '0');
            }
            if (halfmove > 100) throw std::invalid_argument("FEN halfmove clock exceeds 100");
            metadata |= (halfmove << 9) & HALF_MOVE_CLOCK;
        }

        // --- Field 6: Fullmove number (parsed for validation only, not stored) ---
        if (fieldCount >= 6) {
            for (char c : fields[5]) {
                if (c < '0' || c > '9')
                    throw std::invalid_argument("FEN fullmove number is not a valid integer");
            }
        }

        // --- Zobrist hash construction ---
        {
            uint64_t hash = 0;

            // Pieces
            for (int piece = 0; piece < 6; piece++) {
                uint64_t bb = whiteBitboards[piece];
                while (bb) {
                    uint8_t sq  = static_cast<uint8_t>(platform::tzcnt(bb));
                    bb         &= bb - 1;
                    hash       ^= zobrist.pieces[piece][sq];
                }

                bb = blackBitboards[piece];
                while (bb) {
                    uint8_t sq  = static_cast<uint8_t>(platform::tzcnt(bb));
                    bb         &= bb - 1;
                    hash       ^= zobrist.pieces[piece + 6][sq];
                }
            }

            // Side to move
            if (isBlackTurn()) {
                hash ^= zobrist.sideToMove;
            }

            // Castling rights (encode as 4-bit index)
            uint8_t castleIndex = 0;
            if (metadata & WHITE_KINGSIDE_CASTLE) castleIndex |= 1;
            if (metadata & WHITE_QUEENSIDE_CASTLE) castleIndex |= 2;
            if (metadata & BLACK_KINGSIDE_CASTLE) castleIndex |= 4;
            if (metadata & BLACK_QUEENSIDE_CASTLE) castleIndex |= 8;

            hash ^= zobrist.castling[castleIndex];

            // En passant
            if (metadata & EN_PASSANT_VALID) {
                uint8_t file  = metadata & EN_PASSANT_FILE_MASK;
                hash         ^= zobrist.enPassant[file];
            }

            _zobristHash = hash;
        }

        // --- Check detection ---
        // The side that just moved must not be in check. Temporarily flip side to move
        // so that getCheckers() evaluates from the non-moving side's perspective.
        metadata ^= BLACK_TO_MOVE;
        if (platform::popcnt(getCheckers() != 0))
            throw std::invalid_argument("FEN has the non-moving side in check");
        metadata ^= BLACK_TO_MOVE;
    }

    Board::Board() : Board(detail::STARTING_POSITION_FEN) {}

}  // namespace aleph::chess