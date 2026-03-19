#include <gtest/gtest.h>

#include <aleph/chess/square.hpp>

using namespace aleph::chess;

TEST(SquareTest, RankFileRoundtrip) {
    for (uint8_t r = 0; r < 8; r++)
        for (uint8_t f = 0; f < 8; f++) {
            Square sq(r, f);
            EXPECT_EQ(sq.rank(), r);
            EXPECT_EQ(sq.file(), f);
        }
}

TEST(SquareTest, IndexRoundtrip) {
    for (uint8_t i = 0; i < 64; i++) {
        Square sq(i);
        EXPECT_EQ(static_cast<uint8_t>(sq), i);
    }
}

TEST(SquareTest, RankFileConstructorMatchesIndex) {
    for (uint8_t i = 0; i < 64; i++) {
        Square byIndex(i);
        Square byRankFile(i / 8, i % 8);
        EXPECT_EQ(static_cast<uint8_t>(byIndex), static_cast<uint8_t>(byRankFile));
    }
}

TEST(SquareTest, DefaultConstructorIsZero) {
    Square sq;
    EXPECT_EQ(static_cast<uint8_t>(sq), 0);
}

#ifndef NDEBUG
TEST(SquareTest, OutOfRangeAsserts) {
    EXPECT_DEATH(Square(64), "");
    EXPECT_DEATH(Square(8, 0), "");
    EXPECT_DEATH(Square(0, 8), "");
}
#endif

TEST(SquareTest, DirectionValues) {
    EXPECT_EQ(TOWARDS_A_FILE, -1);
    EXPECT_EQ(TOWARDS_H_FILE,  1);
    EXPECT_EQ(TOWARDS_1_RANK, -8);
    EXPECT_EQ(TOWARDS_8_RANK,  8);
}

TEST(SquareTest, FormatterAlgebraic) {
    EXPECT_EQ(fmt::format("{}", Square(0, 0)), "a1");
    EXPECT_EQ(fmt::format("{}", Square(7, 7)), "h8");
    EXPECT_EQ(fmt::format("{}", Square(3, 4)), "e4");
}

TEST(SquareTest, FormatterInString) {
    Square sq(1, 4);
    EXPECT_EQ(fmt::format("square is {}", sq), "square is e2");
}