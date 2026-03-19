/**
 * @file tests/test_move.cpp
 * 
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <gtest/gtest.h>

#include <aleph/chess/move.hpp>

using namespace aleph::chess;

TEST(MoveTest, FromTo) {
    Move m(Square(1, 4), Square(3, 4));
    EXPECT_EQ(m.from(), Square(1, 4));
    EXPECT_EQ(m.to(), Square(3, 4));
}

TEST(MoveTest, NoPromo) {
    Move m(Square(1, 4), Square(3, 4));
    EXPECT_FALSE(m.hasPromo());
    EXPECT_EQ(m.promo(), PAWN);
}

TEST(MoveTest, Promo) {
    Move m(Square(6, 4), Square(7, 4), QUEEN);
    EXPECT_TRUE(m.hasPromo());
    EXPECT_EQ(m.promo(), QUEEN);
}

#ifndef NDEBUG
TEST(MoveTest, InvalidPromoAsserts) {
    EXPECT_DEATH(Move(Square(6, 4), Square(7, 4), KING), "");
    EXPECT_DEATH(Move(Square(6, 4), Square(7, 4), NONE), "");
}
#endif

TEST(MoveTest, Uint16Roundtrip) {
    Move m(Square(1, 4), Square(3, 4));
    EXPECT_EQ(static_cast<uint16_t>(Move(Square(1, 4), Square(3, 4))), static_cast<uint16_t>(m));
}

TEST(MoveTest, FormatterNoPromo) {
    EXPECT_EQ(fmt::format("{}", Move(Square(1, 4), Square(3, 4))), "e2e4");
}

TEST(MoveTest, FormatterPromo) {
    EXPECT_EQ(fmt::format("{}", Move(Square(6, 4), Square(7, 4), QUEEN)), "e7e8q");
}

// Encoding integrity — from/to don't bleed into each other's bits
TEST(MoveTest, FromToIndependent) {
    for (uint8_t i = 0; i < 64; i++) {
        for (uint8_t j = 0; j < 64; j++) {
            Move m(i, j);
            EXPECT_EQ(m.from(), i);
            EXPECT_EQ(m.to(), j);
        }
    }
}

// Promo field doesn't corrupt from/to
TEST(MoveTest, PromoDoesNotCorruptSquares) {
    for (auto type : {BISHOP, KNIGHT, ROOK, QUEEN}) {
        Move m(Square(6, 0), Square(7, 0), type);
        EXPECT_EQ(static_cast<uint8_t>(m.from()), static_cast<uint8_t>(Square(6, 0)));
        EXPECT_EQ(static_cast<uint8_t>(m.to()), static_cast<uint8_t>(Square(7, 0)));
        EXPECT_EQ(m.promo(), type);
    }
}

// All valid promotion types
TEST(MoveTest, AllPromoTypes) {
    for (auto type : {BISHOP, KNIGHT, ROOK, QUEEN}) {
        Move m(Square(6, 4), Square(7, 4), type);
        EXPECT_TRUE(m.hasPromo());
        EXPECT_EQ(m.promo(), type);
    }
}

// Formatter for all promotion types
TEST(MoveTest, FormatterAllPromos) {
    EXPECT_EQ(fmt::format("{}", Move(Square(6, 4), Square(7, 4), BISHOP)), "e7e8b");
    EXPECT_EQ(fmt::format("{}", Move(Square(6, 4), Square(7, 4), KNIGHT)), "e7e8n");
    EXPECT_EQ(fmt::format("{}", Move(Square(6, 4), Square(7, 4), ROOK)), "e7e8r");
    EXPECT_EQ(fmt::format("{}", Move(Square(6, 4), Square(7, 4), QUEEN)), "e7e8q");
}

// Same from/to squares are valid (null move)
TEST(MoveTest, NullMove) {
    Move m(Square(0), Square(0));
    EXPECT_EQ(static_cast<uint8_t>(m.from()), 0);
    EXPECT_EQ(static_cast<uint8_t>(m.to()), 0);
    EXPECT_FALSE(m.hasPromo());
}

// Extreme squares
TEST(MoveTest, CornerSquares) {
    Move m(Square(0), Square(63));
    EXPECT_EQ(static_cast<uint8_t>(m.from()), 0);
    EXPECT_EQ(static_cast<uint8_t>(m.to()), 63);
}

TEST(MoveTest, ToStringNoPromo) { EXPECT_EQ(Move(Square(1, 4), Square(3, 4)).toString(), "e2e4"); }

TEST(MoveTest, ToStringPromo) {
    EXPECT_EQ(Move(Square(6, 4), Square(7, 4), QUEEN).toString(), "e7e8q");
}

TEST(MoveTest, ToStringAllPromos) {
    EXPECT_EQ(Move(Square(6, 4), Square(7, 4), BISHOP).toString(), "e7e8b");
    EXPECT_EQ(Move(Square(6, 4), Square(7, 4), KNIGHT).toString(), "e7e8n");
    EXPECT_EQ(Move(Square(6, 4), Square(7, 4), ROOK).toString(), "e7e8r");
    EXPECT_EQ(Move(Square(6, 4), Square(7, 4), QUEEN).toString(), "e7e8q");
}

TEST(MoveTest, ToStringCornerSquares) { EXPECT_EQ(Move(Square(0), Square(63)).toString(), "a1h8"); }

TEST(MoveTest, ToStringMatchesFormatter) {
    Move m(Square(1, 4), Square(3, 4));
    EXPECT_EQ(m.toString(), fmt::format("{}", m));
}