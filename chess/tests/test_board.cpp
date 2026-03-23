/**
 * @file tests/test_board.cpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <gtest/gtest.h>

#include <aleph/chess/board.hpp>

using namespace aleph::chess;

static Move makeAlgebraicMove(std::string_view from, std::string_view to) {
    uint8_t fromFile = from[0] - 'a';
    uint8_t fromRank = from[1] - '1';
    uint8_t toFile   = to[0] - 'a';
    uint8_t toRank   = to[1] - '1';
    return Move(Square(fromRank, fromFile), Square(toRank, toFile));
}

static Move makePromoMove(std::string_view from, std::string_view to, PieceType promo) {
    uint8_t fromFile = from[0] - 'a';
    uint8_t fromRank = from[1] - '1';
    uint8_t toFile   = to[0] - 'a';
    uint8_t toRank   = to[1] - '1';
    return Move(Square(fromRank, fromFile), Square(toRank, toFile), promo);
}

static bool isLegal(const Board& b, std::string_view from, std::string_view to) {
    return b.getLegalMoves().contains(makeAlgebraicMove(from, to));
}

static bool isLegalPromo(const Board& b, std::string_view from, std::string_view to,
                         PieceType promo) {
    return b.getLegalMoves().contains(makePromoMove(from, to, promo));
}

// --- FEN constructor ---

TEST(BoardFenTest, DefaultConstructor) { EXPECT_NO_THROW(Board()); }

TEST(BoardFenTest, StartingPosition) {
    EXPECT_NO_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
}

TEST(BoardFenTest, SideToMoveWhite) {
    Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    EXPECT_FALSE(b.isBlackTurn());
    EXPECT_TRUE(b.isWhiteTurn());
}

TEST(BoardFenTest, SideToMoveBlack) {
    Board b("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    EXPECT_TRUE(b.isBlackTurn());
    EXPECT_FALSE(b.isWhiteTurn());
}

TEST(BoardFenTest, NoCastlingRights) {
    EXPECT_NO_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1"));
}

TEST(BoardFenTest, PartialCastlingRights) {
    EXPECT_NO_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w Kq - 0 1"));
}

TEST(BoardFenTest, EnPassantValid) {
    EXPECT_NO_THROW(Board("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"));
}

TEST(BoardFenTest, MinimalFourFields) {
    EXPECT_NO_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"));
}

TEST(BoardFenTest, KiwipetePosition) {
    EXPECT_NO_THROW(Board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"));
}

TEST(BoardFenTest, TooFewFields) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq"),
                 std::invalid_argument);
}

TEST(BoardFenTest, TooManyRanks) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, TooFewRanks) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, RankExceeds8Squares) {
    EXPECT_THROW(Board("rnbqkbnrr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, RankUnder8Squares) {
    EXPECT_THROW(Board("rnbqkbn/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, InvalidPieceCharacter) {
    EXPECT_THROW(Board("rnbqkbnx/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, MissingWhiteKing) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQQBNR w KQkq - 0 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, MissingBlackKing) {
    EXPECT_THROW(Board("rnbqqbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, TwoWhiteKings) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKKNR w KQkq - 0 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, PawnOnRank1) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNP w KQkq - 0 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, PawnOnRank8) {
    EXPECT_THROW(Board("rnbqkbnP/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, TooManyWhitePawns) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/PPPPPPPPP/8/RNBQKBNR w KQkq - 0 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, CastlingWhiteKingsideNoRook) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBN1 w K - 0 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, CastlingWhiteKingsideNoKing) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQ1BNR w K - 0 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, CastlingBlackQueensideNoRook) {
    EXPECT_THROW(Board("1nbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w q - 0 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, InvalidCastlingCharacter) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w X - 0 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, EnPassantWrongRankForBlackToMove) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e6 0 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, EnPassantWrongRankForWhiteToMove) {
    EXPECT_THROW(Board("rnbqkbnr/pppp1ppp/8/4p3/8/8/PPPPPPPP/RNBQKBNR w KQkq e3 0 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, EnPassantNoPawnPresent) {
    EXPECT_THROW(Board("rnbqkbnr/pppp1ppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq e3 0 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, EnPassantInvalidFile) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq x3 0 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, HalfmoveClockValid) {
    EXPECT_NO_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 42 1"));
}

TEST(BoardFenTest, HalfmoveClockExceeds100) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 101 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, HalfmoveClockInvalidCharacter) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - x 1"),
                 std::invalid_argument);
}

TEST(BoardFenTest, FullmoveInvalidCharacter) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 x"),
                 std::invalid_argument);
}

// --- push() ---

TEST(BoardPushTest, FlipsSideToMove) {
    Board b;
    EXPECT_FALSE(b.isBlackTurn());
    Board b2 = b.push(makeAlgebraicMove("e2", "e4"));
    EXPECT_TRUE(b2.isBlackTurn());
    Board b3 = b2.push(makeAlgebraicMove("e7", "e5"));
    EXPECT_FALSE(b3.isBlackTurn());
}

TEST(BoardPushTest, WhitePawnSinglePush) {
    Board b;
    Board b2 = b.push(makeAlgebraicMove("e2", "e3"));
    EXPECT_EQ(b2.get(Square(2, 4)).type(), PAWN);
    EXPECT_EQ(b2.get(Square(1, 4)).type(), NONE);
}

TEST(BoardPushTest, WhitePawnDoublePush) {
    Board b;
    Board b2 = b.push(makeAlgebraicMove("e2", "e4"));
    EXPECT_EQ(b2.get(Square(3, 4)).type(), PAWN);
    EXPECT_EQ(b2.get(Square(1, 4)).type(), NONE);
}

TEST(BoardPushTest, BlackPawnSinglePush) {
    Board b("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    Board b2 = b.push(makeAlgebraicMove("e7", "e6"));
    EXPECT_EQ(b2.get(Square(5, 4)).type(), PAWN);
    EXPECT_EQ(b2.get(Square(6, 4)).type(), NONE);
}

TEST(BoardPushTest, BlackPawnDoublePush) {
    Board b("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    Board b2 = b.push(makeAlgebraicMove("e7", "e5"));
    EXPECT_EQ(b2.get(Square(4, 4)).type(), PAWN);
    EXPECT_EQ(b2.get(Square(6, 4)).type(), NONE);
}

TEST(BoardPushTest, WhiteDoublePushSetsEnPassant) {
    Board b;
    Board b2 = b.push(makeAlgebraicMove("e2", "e4"));
    EXPECT_TRUE(b2.isEnPassantValid());
    EXPECT_EQ(b2.getEnPassantFile(), 4);
}

TEST(BoardPushTest, BlackDoublePushSetsEnPassant) {
    Board b("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1");
    Board b2 = b.push(makeAlgebraicMove("d7", "d5"));
    EXPECT_TRUE(b2.isEnPassantValid());
    EXPECT_EQ(b2.getEnPassantFile(), 3);
}

TEST(BoardPushTest, NonDoublePushClearsEnPassant) {
    Board b;
    Board b2 = b.push(makeAlgebraicMove("e2", "e4"));
    EXPECT_TRUE(b2.isEnPassantValid());
    Board b3 = b2.push(makeAlgebraicMove("e7", "e6"));
    EXPECT_FALSE(b3.isEnPassantValid());
}

TEST(BoardPushTest, WhiteEnPassantCapture) {
    Board b("rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 1");
    Board b2 = b.push(makeAlgebraicMove("e5", "d6"));
    EXPECT_EQ(b2.get(Square(5, 3)).type(), PAWN);
    EXPECT_EQ(b2.get(Square(4, 3)).type(), NONE);
    EXPECT_EQ(b2.get(Square(4, 4)).type(), NONE);
}

TEST(BoardPushTest, BlackEnPassantCapture) {
    Board b("rnbqkbnr/ppp1pppp/8/8/3pP3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    Board b2 = b.push(makeAlgebraicMove("d4", "e3"));
    EXPECT_EQ(b2.get(Square(2, 4)).type(), PAWN);
    EXPECT_EQ(b2.get(Square(3, 4)).type(), NONE);
    EXPECT_EQ(b2.get(Square(3, 3)).type(), NONE);
}

TEST(BoardPushTest, WhiteCapturesBlackPiece) {
    Board b("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1");
    Board b2 = b.push(makeAlgebraicMove("e4", "d5"));
    EXPECT_EQ(b2.get(Square(4, 3)).type(), PAWN);
    EXPECT_EQ(b2.get(Square(3, 4)).type(), NONE);
}

TEST(BoardPushTest, BlackCapturesWhitePiece) {
    Board b("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1");
    Board b2 = b.push(makeAlgebraicMove("d5", "e4"));
    EXPECT_EQ(b2.get(Square(3, 4)).type(), PAWN);
    EXPECT_EQ(b2.get(Square(4, 3)).type(), NONE);
}

TEST(BoardPushTest, WhitePromotionToQueen) {
    Board b("8/4P3/8/8/8/8/8/4K2k w - - 0 1");
    Board b2 = b.push(makePromoMove("e7", "e8", QUEEN));
    EXPECT_EQ(b2.get(Square(7, 4)).type(), QUEEN);
    EXPECT_EQ(b2.get(Square(6, 4)).type(), NONE);
}

TEST(BoardPushTest, WhitePromotionToKnight) {
    Board b("8/4P3/8/8/8/8/8/4K2k w - - 0 1");
    Board b2 = b.push(makePromoMove("e7", "e8", KNIGHT));
    EXPECT_EQ(b2.get(Square(7, 4)).type(), KNIGHT);
}

TEST(BoardPushTest, BlackPromotionToQueen) {
    Board b("4k3/8/8/8/8/8/4p3/4K3 b - - 0 1");
    Board b2 = b.push(makePromoMove("e2", "e1", QUEEN));
    EXPECT_EQ(b2.get(Square(0, 4)).type(), QUEEN);
    EXPECT_EQ(b2.get(Square(1, 4)).type(), NONE);
}

TEST(BoardPushTest, WhiteKingsideCastle) {
    Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQkq - 0 1");
    Board b2 = b.push(makeAlgebraicMove("e1", "g1"));
    EXPECT_EQ(b2.get(Square(0, 6)).type(), KING);
    EXPECT_EQ(b2.get(Square(0, 5)).type(), ROOK);
    EXPECT_EQ(b2.get(Square(0, 4)).type(), NONE);
    EXPECT_EQ(b2.get(Square(0, 7)).type(), NONE);
}

TEST(BoardPushTest, WhiteQueensideCastle) {
    Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/R3KBNR w KQkq - 0 1");
    Board b2 = b.push(makeAlgebraicMove("e1", "c1"));
    EXPECT_EQ(b2.get(Square(0, 2)).type(), KING);
    EXPECT_EQ(b2.get(Square(0, 3)).type(), ROOK);
    EXPECT_EQ(b2.get(Square(0, 4)).type(), NONE);
    EXPECT_EQ(b2.get(Square(0, 0)).type(), NONE);
}

TEST(BoardPushTest, BlackKingsideCastle) {
    Board b("rnbqk2r/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
    Board b2 = b.push(makeAlgebraicMove("e8", "g8"));
    EXPECT_EQ(b2.get(Square(7, 6)).type(), KING);
    EXPECT_EQ(b2.get(Square(7, 5)).type(), ROOK);
    EXPECT_EQ(b2.get(Square(7, 4)).type(), NONE);
    EXPECT_EQ(b2.get(Square(7, 7)).type(), NONE);
}

TEST(BoardPushTest, BlackQueensideCastle) {
    Board b("r3kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
    Board b2 = b.push(makeAlgebraicMove("e8", "c8"));
    EXPECT_EQ(b2.get(Square(7, 2)).type(), KING);
    EXPECT_EQ(b2.get(Square(7, 3)).type(), ROOK);
    EXPECT_EQ(b2.get(Square(7, 4)).type(), NONE);
    EXPECT_EQ(b2.get(Square(7, 0)).type(), NONE);
}

TEST(BoardPushTest, WhiteKingMoveClearsBothWhiteRights) {
    Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQkq - 0 1");
    Board b2 = b.push(makeAlgebraicMove("e1", "g1"));
    EXPECT_FALSE(b2.canWhiteKingsideCastle());
    EXPECT_FALSE(b2.canWhiteQueensideCastle());
}

TEST(BoardPushTest, BlackKingMoveClearsBothBlackRights) {
    Board b("rnbqk2r/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
    Board b2 = b.push(makeAlgebraicMove("e8", "g8"));
    EXPECT_FALSE(b2.canBlackKingsideCastle());
    EXPECT_FALSE(b2.canBlackQueensideCastle());
}

TEST(BoardPushTest, WhiteKingsideRookMoveClearsKingsideRight) {
    Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQkq - 0 1");
    Board b2 = b.push(makeAlgebraicMove("h1", "g1"));
    EXPECT_FALSE(b2.canWhiteKingsideCastle());
    EXPECT_TRUE(b2.canWhiteQueensideCastle());
}

TEST(BoardPushTest, WhiteQueensideRookMoveClearsQueensideRight) {
    Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/R3KBNR w KQkq - 0 1");
    Board b2 = b.push(makeAlgebraicMove("a1", "b1"));
    EXPECT_TRUE(b2.canWhiteKingsideCastle());
    EXPECT_FALSE(b2.canWhiteQueensideCastle());
}

TEST(BoardPushTest, BlackRookCapturedOnH8ClearsKingsideRight) {
    Board b("rnbqk2r/pppppppP/8/8/8/8/PPPPPPP1/RNBQKBNR w KQkq - 0 1");
    Board b2 = b.push(makePromoMove("h7", "h8", QUEEN));
    EXPECT_FALSE(b2.canBlackKingsideCastle());
}

TEST(BoardPushTest, BlackRookCapturedOnA8ClearsQueensideRight) {
    Board b("r3kbnr/Pppppppp/8/8/8/8/1PPPPPPP/RNBQKBNR w KQkq - 0 1");
    Board b2 = b.push(makePromoMove("a7", "a8", QUEEN));
    EXPECT_FALSE(b2.canBlackQueensideCastle());
}

TEST(BoardPushTest, HalfmoveClockIncrements) {
    Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    Board b2 = b.push(makeAlgebraicMove("g1", "f3"));
    EXPECT_EQ(b2.getHalfMoveClock(), 1);
}

TEST(BoardPushTest, HalfmoveClockResetsOnPawnMove) {
    Board b("rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R w KQkq - 5 1");
    Board b2 = b.push(makeAlgebraicMove("e2", "e4"));
    EXPECT_EQ(b2.getHalfMoveClock(), 0);
}

TEST(BoardPushTest, HalfmoveClockResetsOnCapture) {
    Board b("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 5 1");
    Board b2 = b.push(makeAlgebraicMove("e4", "d5"));
    EXPECT_EQ(b2.getHalfMoveClock(), 0);
}

TEST(BoardPushTest, OccupancyUpdatesAfterMove) {
    Board b;
    Board b2 = b.push(makeAlgebraicMove("e2", "e4"));
    EXPECT_TRUE(b2.getOccupancy() & (1ULL << static_cast<uint8_t>(Square(3, 4))));
    EXPECT_FALSE(b2.getOccupancy() & (1ULL << static_cast<uint8_t>(Square(1, 4))));
}

TEST(BoardPushTest, OccupancyUpdatesAfterCapture) {
    Board b("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1");
    Board b2     = b.push(makeAlgebraicMove("e4", "d5"));
    uint64_t occ = b2.getOccupancy();
    EXPECT_TRUE(occ & (1ULL << static_cast<uint8_t>(Square(4, 3))));
    EXPECT_FALSE(occ & (1ULL << static_cast<uint8_t>(Square(3, 4))));
}

TEST(BoardPushTest, OriginalBoardUnchangedAfterPush) {
    Board b;
    uint64_t occBefore = b.getOccupancy();
    (void)b.push(makeAlgebraicMove("e2", "e4"));
    EXPECT_EQ(b.getOccupancy(), occBefore);
}

TEST(BoardPushTest, KnightMoveCorrect) {
    Board b;
    Board b2 = b.push(makeAlgebraicMove("g1", "f3"));
    EXPECT_EQ(b2.get(Square(2, 5)).type(), KNIGHT);
    EXPECT_EQ(b2.get(Square(0, 6)).type(), NONE);
}

TEST(BoardPushTest, BishopMoveCorrect) {
    Board b("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1");
    Board b2 = b.push(makeAlgebraicMove("f1", "c4"));
    EXPECT_EQ(b2.get(Square(3, 2)).type(), BISHOP);
    EXPECT_EQ(b2.get(Square(0, 5)).type(), NONE);
}

TEST(BoardPushTest, RookMoveCorrect) {
    Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w K - 0 1");
    Board b2 = b.push(makeAlgebraicMove("h1", "f1"));
    EXPECT_EQ(b2.get(Square(0, 5)).type(), ROOK);
    EXPECT_EQ(b2.get(Square(0, 7)).type(), NONE);
}

TEST(BoardPushTest, QueenMoveCorrect) {
    Board b("rnbqkbnr/pppppppp/8/8/8/4P3/PPPP1PPP/RNBQKBNR w KQkq - 0 1");
    Board b2 = b.push(makeAlgebraicMove("d1", "h5"));
    EXPECT_EQ(b2.get(Square(4, 7)).type(), QUEEN);
    EXPECT_EQ(b2.get(Square(0, 3)).type(), NONE);
}

// --- isLegalFast (tested via getLegalMoves) ---

TEST(IsLegalFastTest, StartingPositionMoveCount) {
    Board b;
    EXPECT_EQ(b.getLegalMoves().size(), 20);
}

TEST(IsLegalFastTest, PawnSinglePushLegal) {
    Board b;
    EXPECT_TRUE(isLegal(b, "e2", "e3"));
}

TEST(IsLegalFastTest, PawnDoublePushLegal) {
    Board b;
    EXPECT_TRUE(isLegal(b, "e2", "e4"));
}

TEST(IsLegalFastTest, KnightMoveFromStartLegal) {
    Board b;
    EXPECT_TRUE(isLegal(b, "g1", "f3"));
    EXPECT_TRUE(isLegal(b, "b1", "c3"));
}

TEST(IsLegalFastTest, PinnedPieceCannotMoveOffRay) {
    Board b("4k3/4r3/8/8/8/8/4B3/4K3 w - - 0 1");
    EXPECT_FALSE(isLegal(b, "e2", "d3"));
    EXPECT_FALSE(isLegal(b, "e2", "f3"));
}

TEST(IsLegalFastTest, PinnedPieceCanMoveAlongRay) {
    Board b("4k3/4r3/8/8/8/8/4R3/4K3 w - - 0 1");
    EXPECT_TRUE(isLegal(b, "e2", "e3"));
    EXPECT_TRUE(isLegal(b, "e2", "e4"));
    EXPECT_TRUE(isLegal(b, "e2", "e7"));
}

TEST(IsLegalFastTest, DiagonalPinCannotMoveOffDiagonal) {
    Board b("8/8/6b1/8/8/3B4/8/1K5k w - - 0 1");
    EXPECT_FALSE(isLegal(b, "d3", "e2"));
    EXPECT_FALSE(isLegal(b, "d3", "f1"));
    EXPECT_FALSE(isLegal(b, "d3", "b5"));
}

TEST(IsLegalFastTest, DiagonalPinCanMoveAlongDiagonal) {
    Board b("8/8/6b1/8/8/3B4/8/1K5k w - - 0 1");
    EXPECT_TRUE(isLegal(b, "d3", "c2"));
    EXPECT_TRUE(isLegal(b, "d3", "e4"));
    EXPECT_TRUE(isLegal(b, "d3", "g6"));
}

TEST(IsLegalFastTest, MustEvadeCheck) {
    Board b("4r3/8/8/8/8/8/8/4K2k w - - 0 1");
    EXPECT_FALSE(isLegal(b, "e1", "e2"));
    EXPECT_TRUE(isLegal(b, "e1", "f1"));
    EXPECT_TRUE(isLegal(b, "e1", "d1"));
    EXPECT_TRUE(isLegal(b, "e1", "d2"));
    EXPECT_TRUE(isLegal(b, "e1", "f2"));
}

TEST(IsLegalFastTest, CanBlockCheck) {
    Board b("4r3/8/8/8/R7/8/8/4K2k w - - 0 1");
    EXPECT_TRUE(isLegal(b, "a4", "e4"));
}

TEST(IsLegalFastTest, CanCaptureChecker) {
    Board b("4k3/8/8/8/8/8/4r3/R3K3 w - - 0 1");
    EXPECT_TRUE(isLegal(b, "e1", "e2"));
    EXPECT_FALSE(isLegal(b, "e1", "d2"));
    EXPECT_FALSE(isLegal(b, "e1", "f2"));
}

TEST(IsLegalFastTest, DoubleCheckOnlyKingCanMove) {
    Board b("4k3/8/8/8/b7/8/4r3/4K3 w - - 0 1");
    auto moves = b.getLegalMoves();
    for (auto m : moves) EXPECT_EQ(b.get(m.from()).type(), KING);
}

TEST(IsLegalFastTest, KingCannotMoveIntoCheck) {
    Board b("5r2/8/8/8/8/8/8/4K2k w - - 0 1");
    EXPECT_FALSE(isLegal(b, "e1", "f1"));
    EXPECT_FALSE(isLegal(b, "e1", "f2"));
}

TEST(IsLegalFastTest, KingCannotMoveAdjacentToEnemyKing) {
    Board b("8/8/8/8/8/8/8/4K1k1 w - - 0 1");
    EXPECT_FALSE(isLegal(b, "e1", "f1"));
    EXPECT_FALSE(isLegal(b, "e1", "f2"));
}

TEST(IsLegalFastTest, KingCanCaptureUndefendedPiece) {
    Board b("8/8/8/8/8/8/5r2/4K2k w - - 0 1");
    EXPECT_TRUE(isLegal(b, "e1", "f2"));
}

TEST(IsLegalFastTest, KingCannotCaptureDefendedPiece) {
    Board b("5r2/8/8/8/8/8/5r2/4K2k w - - 0 1");
    EXPECT_FALSE(isLegal(b, "e1", "f2"));
}

TEST(IsLegalFastTest, CastlingLegalWhenClear) {
    Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQkq - 0 1");
    EXPECT_TRUE(isLegal(b, "e1", "g1"));
}

TEST(IsLegalFastTest, CastlingIllegalWhenInCheck) {
    Board b("4rk2/8/8/8/8/8/8/RNBQK2R w KQ - 0 1");
    EXPECT_FALSE(isLegal(b, "e1", "g1"));
}

TEST(IsLegalFastTest, CastlingIllegalWhenPassingThroughCheck) {
    Board b("5rk1/8/8/8/8/8/8/RNBQK2R w KQ - 0 1");
    EXPECT_FALSE(isLegal(b, "e1", "g1"));
}

TEST(IsLegalFastTest, CastlingIllegalWhenLandingInCheck) {
    Board b("5kr1/8/8/8/8/8/8/RNBQK2R w KQ - 0 1");
    EXPECT_FALSE(isLegal(b, "e1", "g1"));
}

TEST(IsLegalFastTest, EnPassantLegal) {
    Board b("rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 1");
    EXPECT_TRUE(isLegal(b, "e5", "d6"));
}

TEST(IsLegalFastTest, EnPassantIllegalIfExposesDiagonalCheck) {
    Board b("k7/1b6/8/3Pp3/8/8/6K1/8 w - - 0 1");
    EXPECT_FALSE(isLegal(b, "d5", "e6"));
}

TEST(IsLegalFastTest, EnPassantIllegalIfExposesRankCheck) {
    Board b("8/8/8/KPp3rk/8/8/8/8 w - c6 0 1");
    EXPECT_FALSE(isLegal(b, "b5", "c6"));
}

TEST(IsLegalFastTest, PromotionLegal) {
    Board b("k7/4P3/8/8/8/8/8/4K3 w - - 0 1");
    EXPECT_TRUE(isLegalPromo(b, "e7", "e8", QUEEN));
    EXPECT_TRUE(isLegalPromo(b, "e7", "e8", ROOK));
    EXPECT_TRUE(isLegalPromo(b, "e7", "e8", BISHOP));
    EXPECT_TRUE(isLegalPromo(b, "e7", "e8", KNIGHT));
}

TEST(IsLegalFastTest, PromotionIllegalIfLeavesKingInCheck) {
    Board b("K6k/1P6/2b5/8/8/8/8/8 w - - 0 1");
    EXPECT_FALSE(isLegalPromo(b, "b7", "b8", QUEEN));
}

TEST(IsLegalFastTest, PerftStartingPositionDepth1) {
    Board b;
    EXPECT_EQ(b.getLegalMoves().size(), 20);
}

TEST(IsLegalFastTest, PerftKiwipeteDepth1) {
    Board b("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
    EXPECT_EQ(b.getLegalMoves().size(), 48);
}

TEST(IsLegalFastTest, PerftPosition3Depth1) {
    Board b("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");
    EXPECT_EQ(b.getLegalMoves().size(), 14);
}