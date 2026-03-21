#pragma once

#include <functional>
#include <numeric>

#include <aleph/platform.hpp>

#include "board.hpp"

namespace aleph::chess {
    Board::Board(std::string_view fen)
        : whiteBitboards{},
          blackBitboards{},
          metadata(0),
          _cacheValid(0),
          _occupancy(0),
          _whiteOccupancy(0),
          _blackOccupancy(0),
          _zobristHash(0) {
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
            int rankCount       = 0;

            for (char c : fields[0]) {
                if (c == '/') {
                    if (fileIdx != 8)
                        throw std::invalid_argument("FEN rank does not sum to 8 squares");
                    if (rankIdx == 0) throw std::invalid_argument("FEN has too many ranks");
                    --rankIdx;
                    fileIdx = 0;
                    ++rankCount;
                } else if (c >= '1' && c <= '8') {
                    fileIdx += static_cast<std::size_t>(c - '0');
                    if (fileIdx > 8) throw std::invalid_argument("FEN rank exceeds 8 squares");
                } else {
                    // Validate it's a known piece character before constructing
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

            // Final rank check
            if (fileIdx != 8)
                throw std::invalid_argument("FEN final rank does not sum to 8 squares");
            if (rankIdx != 0) throw std::invalid_argument("FEN has too few ranks");
        }

        // --- Piece validity checks ---
        {
            // King counts
            auto whiteKings = platform::popcnt(whiteBitboards[KING]);
            auto blackKings = platform::popcnt(blackBitboards[KING]);
            if (whiteKings != 1)
                throw std::invalid_argument("FEN must have exactly one white king");
            if (blackKings != 1)
                throw std::invalid_argument("FEN must have exactly one black king");

            // Pawn counts
            auto whitePawns = platform::popcnt(whiteBitboards[PAWN]);
            auto blackPawns = platform::popcnt(blackBitboards[PAWN]);
            if (whitePawns > 8) throw std::invalid_argument("FEN has too many white pawns");
            if (blackPawns > 8) throw std::invalid_argument("FEN has too many black pawns");

            // Pawns on back ranks
            constexpr uint64_t RANK_1 = 0x00000000000000FFULL;
            constexpr uint64_t RANK_8 = 0xFF00000000000000ULL;
            if (whiteBitboards[PAWN] & (RANK_1 | RANK_8))
                throw std::invalid_argument("FEN has pawns on back rank");
            if (blackBitboards[PAWN] & (RANK_1 | RANK_8))
                throw std::invalid_argument("FEN has pawns on back rank");

            // Total piece counts
            auto whitePieces = 0;
            auto blackPieces = 0;
            for (int i = 0; i < 6; i++) {
                whitePieces += platform::popcnt(whiteBitboards[i]);
                blackPieces += platform::popcnt(blackBitboards[i]);
            }
            if (whitePieces > 16) throw std::invalid_argument("FEN has too many white pieces");
            if (blackPieces > 16) throw std::invalid_argument("FEN has too many black pieces");

            // Pieces cannot overlap
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

                bool blackToMove = metadata & BLACK_TO_MOVE;

                // En passant rank must be 3 (after white double push, black to move)
                // or 6 (after black double push, white to move) — 0-indexed
                if (blackToMove && rank != 2)
                    throw std::invalid_argument(
                        "FEN en passant rank must be 3 when black is to move");
                if (!blackToMove && rank != 5)
                    throw std::invalid_argument(
                        "FEN en passant rank must be 6 when white is to move");

                // There must be a pawn that made the double push
                uint64_t pawnBit = blackToMove ? 1ULL << static_cast<uint8_t>(Square(3, file))
                                               : 1ULL << static_cast<uint8_t>(Square(4, file));

                if (blackToMove && !(whiteBitboards[PAWN] & pawnBit))
                    throw std::invalid_argument(
                        "FEN en passant square is invalid: no white pawn on expected square");
                if (!blackToMove && !(blackBitboards[PAWN] & pawnBit))
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

        // --- Field 6: Fullmove number (ignored, no storage) ---
        if (fieldCount >= 6) {
            for (char c : fields[5]) {
                if (c < '0' || c > '9')
                    throw std::invalid_argument("FEN fullmove number is not a valid integer");
            }
        }

        // --- Check detection: side not to move must not be in check ---
        // Deferred until isLegalFast is implemented.
    }

    inline Piece Board::get(Square sq) const {
        uint64_t bit = 1ULL << static_cast<uint8_t>(sq);
        for (int i = 0; i < 6; i++) {
            if (whiteBitboards[i] & bit) return Piece(PieceType(i), false);
            if (blackBitboards[i] & bit) return Piece(PieceType(i), true);
        }
        return Piece(NONE, false);
    }

    Board Board::push(Move m) const {
        // DEBUG_ASSERT(isLegal(m));

        Board next            = *this;
        next._cacheValid      = 0;

        Square from           = m.from();
        Square to             = m.to();

        uint64_t fromBit      = 1ULL << static_cast<uint8_t>(from);
        uint64_t toBit        = 1ULL << static_cast<uint8_t>(to);

        bool isBlack          = next.metadata & BLACK_TO_MOVE;

        auto& ownBitboards    = isBlack ? next.blackBitboards : next.whiteBitboards;
        auto& enemyBitboards  = isBlack ? next.whiteBitboards : next.blackBitboards;

        // --- Determine moving piece type ---
        PieceType movingPiece = NONE;
        for (int i = 0; i < 6; i++) {
            if (ownBitboards[i] & fromBit) {
                movingPiece = PieceType(i);
                break;
            }
        }
        DEBUG_ASSERT(movingPiece != NONE);

        // --- Clear en passant state ---
        next.metadata &= ~(EN_PASSANT_FILE_MASK | EN_PASSANT_VALID);

        // --- Remove captured piece (if any) ---
        for (int i = 0; i < 6; i++) enemyBitboards[i] &= ~toBit;

        // --- Move the piece ---
        ownBitboards[movingPiece] &= ~fromBit;

        // --- Handle promotion ---
        if (m.hasPromo())
            ownBitboards[m.promo()] |= toBit;
        else
            ownBitboards[movingPiece] |= toBit;

        // --- Handle castling (king moves two squares) ---
        if (movingPiece == KING) {
            int8_t fileDelta = static_cast<int8_t>(to.file()) - static_cast<int8_t>(from.file());
            if (fileDelta == 2) {
                // Kingside — move rook from h-file to f-file
                Square rookFrom(from.rank(), 7);
                Square rookTo(from.rank(), 5);
                ownBitboards[ROOK] &= ~(1ULL << static_cast<uint8_t>(rookFrom));
                ownBitboards[ROOK] |= (1ULL << static_cast<uint8_t>(rookTo));
            } else if (fileDelta == -2) {
                // Queenside — move rook from a-file to d-file
                Square rookFrom(from.rank(), 0);
                Square rookTo(from.rank(), 3);
                ownBitboards[ROOK] &= ~(1ULL << static_cast<uint8_t>(rookFrom));
                ownBitboards[ROOK] |= (1ULL << static_cast<uint8_t>(rookTo));
            }
        }

        // --- Handle en passant capture ---
        if (movingPiece == PAWN) {
            // Double push — set en passant file
            int8_t rankDelta = static_cast<int8_t>(to.rank()) - static_cast<int8_t>(from.rank());
            if (rankDelta == 2 || rankDelta == -2) {
                next.metadata |= EN_PASSANT_VALID;
                next.metadata |= (to.file() & EN_PASSANT_FILE_MASK);
            }

            // En passant capture — remove the captured pawn
            if ((metadata & EN_PASSANT_VALID) && to.file() == (metadata & EN_PASSANT_FILE_MASK)) {
                uint8_t capturedRank = isBlack ? to.rank() + 1 : to.rank() - 1;
                Square capturedSq(capturedRank, to.file());
                enemyBitboards[PAWN] &= ~(1ULL << static_cast<uint8_t>(capturedSq));
            }
        }

        // --- Update castling rights ---
        constexpr uint8_t E1 = static_cast<uint8_t>(Square(0, 4));
        constexpr uint8_t E8 = static_cast<uint8_t>(Square(7, 4));
        constexpr uint8_t A1 = static_cast<uint8_t>(Square(0, 0));
        constexpr uint8_t H1 = static_cast<uint8_t>(Square(0, 7));
        constexpr uint8_t A8 = static_cast<uint8_t>(Square(7, 0));
        constexpr uint8_t H8 = static_cast<uint8_t>(Square(7, 7));

        uint8_t fromIdx      = static_cast<uint8_t>(from);
        uint8_t toIdx        = static_cast<uint8_t>(to);

        if (fromIdx == E1) next.metadata &= ~(WHITE_KINGSIDE_CASTLE | WHITE_QUEENSIDE_CASTLE);
        if (fromIdx == E8) next.metadata &= ~(BLACK_KINGSIDE_CASTLE | BLACK_QUEENSIDE_CASTLE);
        if (fromIdx == H1 || toIdx == H1) next.metadata &= ~WHITE_KINGSIDE_CASTLE;
        if (fromIdx == A1 || toIdx == A1) next.metadata &= ~WHITE_QUEENSIDE_CASTLE;
        if (fromIdx == H8 || toIdx == H8) next.metadata &= ~BLACK_KINGSIDE_CASTLE;
        if (fromIdx == A8 || toIdx == A8) next.metadata &= ~BLACK_QUEENSIDE_CASTLE;

        // --- Update halfmove clock ---
        bool isCapture = false;
        for (int i = 0; i < 6; i++) {
            if ((isBlack ? whiteBitboards[i] : blackBitboards[i]) & toBit) {
                isCapture = true;
                break;
            }
        }

        if (movingPiece == PAWN || isCapture) {
            next.metadata &= ~HALF_MOVE_CLOCK;
        } else {
            uint32_t clock = (metadata & HALF_MOVE_CLOCK) >> 9;
            ++clock;
            next.metadata &= ~HALF_MOVE_CLOCK;
            next.metadata |= (clock << 9) & HALF_MOVE_CLOCK;
        }

        // --- Flip side to move ---
        next.metadata ^= BLACK_TO_MOVE;

        return next;
    }

    std::uint64_t Board::occupancy() const {
        if ((_cacheValid & OCCUPANCY_VALID) == 0) {
            _occupancy   = whiteOccupancy() | blackOccupancy();
            _cacheValid |= OCCUPANCY_VALID;
        }
        return _occupancy;
    }

    std::uint64_t Board::whiteOccupancy() const {
        if ((_cacheValid & WHITE_OCCUPANCY_VALID) == 0) {
            _whiteOccupancy  = std::reduce(whiteBitboards.begin(), whiteBitboards.end(), 0ULL,
                                           std::bit_or<uint64_t>{});
            _cacheValid     |= WHITE_OCCUPANCY_VALID;
        }
        return _whiteOccupancy;
    }

    std::uint64_t Board::blackOccupancy() const {
        if ((_cacheValid & BLACK_OCCUPANCY_VALID) == 0) {
            _blackOccupancy  = std::reduce(blackBitboards.begin(), blackBitboards.end(), 0ULL,
                                           std::bit_or<uint64_t>{});
            _cacheValid     |= BLACK_OCCUPANCY_VALID;
        }
        return _blackOccupancy;
    }

    bool Board::isBlackTurn() const { return (metadata & BLACK_TO_MOVE) != 0U; }

    bool Board::isWhiteTurn() const { return (metadata & BLACK_TO_MOVE) == 0U; }

    bool Board::canWhiteKingsideCastle() const { return (metadata & WHITE_KINGSIDE_CASTLE) != 0U; }

    bool Board::canWhiteQueensideCastle() const {
        return (metadata & WHITE_QUEENSIDE_CASTLE) != 0U;
    }

    bool Board::canBlackKingsideCastle() const { return (metadata & BLACK_KINGSIDE_CASTLE) != 0U; }

    bool Board::canBlackQueensideCastle() const {
        return (metadata & BLACK_QUEENSIDE_CASTLE) != 0U;
    }

    bool Board::isEnPassantValid() const { return (metadata & EN_PASSANT_VALID) != 0U; }

    std::uint8_t Board::enPassantFile() const { return (metadata & EN_PASSANT_FILE_MASK); }

    std::uint8_t Board::halfMoveClock() const { return (metadata & HALF_MOVE_CLOCK) >> 9; }
}  // namespace aleph::chess