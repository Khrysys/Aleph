#include <gtest/gtest.h>

#include <aleph/chess/piece.hpp>

using namespace aleph::chess;

TEST(PieceTest, FromCharWhite) {
    EXPECT_EQ(Piece('P').type(), PAWN);
    EXPECT_EQ(Piece('N').type(), KNIGHT);
    EXPECT_EQ(Piece('K').type(), KING);
    EXPECT_TRUE(Piece('P').isWhite());
    EXPECT_FALSE(Piece('P').isBlack());
}

TEST(PieceTest, FromCharBlack) {
    EXPECT_EQ(Piece('p').type(), PAWN);
    EXPECT_EQ(Piece('n').type(), KNIGHT);
    EXPECT_EQ(Piece('k').type(), KING);
    EXPECT_TRUE(Piece('p').isBlack());
    EXPECT_FALSE(Piece('p').isWhite());
}

#ifndef NDEBUG
TEST(PieceTest, FromCharInvalidAsserts) {
    EXPECT_DEATH(Piece('x'), "");
    EXPECT_DEATH(Piece('1'), "");
}
#endif

TEST(PieceTest, TypeRoundtrip) {
    for (auto type : {PAWN, BISHOP, KNIGHT, ROOK, QUEEN, KING}) {
        EXPECT_EQ(Piece(type, false).type(), type);
        EXPECT_EQ(Piece(type, true).type(), type);
    }
}

TEST(PieceTest, ColorEncoding) {
    EXPECT_FALSE(Piece(PAWN, false).isBlack());
    EXPECT_TRUE(Piece(PAWN, true).isBlack());
    EXPECT_FALSE(Piece(KING, false).isBlack());
    EXPECT_TRUE(Piece(KING, true).isBlack());
}

TEST(PieceTest, IndexEncoding) {
    EXPECT_EQ(static_cast<uint8_t>(Piece(PAWN, false)), 0);
    EXPECT_EQ(static_cast<uint8_t>(Piece(KING, false)), 5);
    EXPECT_EQ(static_cast<uint8_t>(Piece(PAWN, true)), 6);
    EXPECT_EQ(static_cast<uint8_t>(Piece(KING, true)), 11);
}

TEST(PieceTest, ToCharBlack) {
    EXPECT_EQ(Piece(PAWN, true).toChar(), 'p');
    EXPECT_EQ(Piece(KNIGHT, true).toChar(), 'n');
    EXPECT_EQ(Piece(BISHOP, true).toChar(), 'b');
    EXPECT_EQ(Piece(ROOK, true).toChar(), 'r');
    EXPECT_EQ(Piece(QUEEN, true).toChar(), 'q');
    EXPECT_EQ(Piece(KING, true).toChar(), 'k');
}

TEST(PieceTest, FormatterWhite) {
    EXPECT_EQ(fmt::format("{}", Piece(PAWN, false)), "P");
    EXPECT_EQ(fmt::format("{}", Piece(KNIGHT, false)), "N");
    EXPECT_EQ(fmt::format("{}", Piece(KING, false)), "K");
}

TEST(PieceTest, FormatterBlack) {
    EXPECT_EQ(fmt::format("{}", Piece(PAWN, true)), "p");
    EXPECT_EQ(fmt::format("{}", Piece(KNIGHT, true)), "n");
    EXPECT_EQ(fmt::format("{}", Piece(KING, true)), "k");
}

#ifndef NDEBUG
TEST(PieceTest, NoneAsserts) {
    EXPECT_DEATH(Piece(NONE, false), "");
    EXPECT_DEATH(Piece(NONE, true), "");
}
#endif