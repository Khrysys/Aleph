/**
 * @file tests/test_move_list.cpp
 * 
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <gtest/gtest.h>

#include <aleph/chess/move_list.hpp>

using namespace aleph::chess;

static Move makeMove(uint8_t from, uint8_t to) { return Move(Square(from), Square(to)); }

TEST(MoveListTest, DefaultEmpty) {
    MoveList<256> ml;
    EXPECT_TRUE(ml.empty());
    EXPECT_EQ(ml.size(), 0);
}

TEST(MoveListTest, Capacity) {
    EXPECT_EQ(MoveList<256>::capacity(), 256);
    EXPECT_EQ(MoveList<512>::capacity(), 512);
}

TEST(MoveListTest, PushBack) {
    MoveList<256> ml;
    ml.push_back(makeMove(0, 1));
    EXPECT_EQ(ml.size(), 1);
    EXPECT_FALSE(ml.empty());
}

TEST(MoveListTest, PushBackMultiple) {
    MoveList<256> ml;
    for (uint8_t i = 0; i < 10; i++) ml.push_back(makeMove(i, i + 1));
    EXPECT_EQ(ml.size(), 10);
}

TEST(MoveListTest, Clear) {
    MoveList<256> ml;
    ml.push_back(makeMove(0, 1));
    ml.clear();
    EXPECT_TRUE(ml.empty());
    EXPECT_EQ(ml.size(), 0);
}

TEST(MoveListTest, IndexAccess) {
    MoveList<256> ml;
    Move m = makeMove(12, 28);
    ml.push_back(m);
    EXPECT_EQ(static_cast<uint16_t>(ml[0]), static_cast<uint16_t>(m));
}

TEST(MoveListTest, Contains) {
    MoveList<256> ml;
    Move m = makeMove(12, 28);
    EXPECT_FALSE(ml.contains(m));
    ml.push_back(m);
    EXPECT_TRUE(ml.contains(m));
}

TEST(MoveListTest, ContainsNotPresent) {
    MoveList<256> ml;
    ml.push_back(makeMove(0, 1));
    EXPECT_FALSE(ml.contains(makeMove(1, 2)));
}

TEST(MoveListTest, OperatorPlusEqMove) {
    MoveList<256> ml;
    ml += makeMove(0, 1);
    EXPECT_EQ(ml.size(), 1);
}

TEST(MoveListTest, OperatorPlusEqList) {
    MoveList<256> a, b;
    a += makeMove(0, 1);
    b += makeMove(2, 3);
    a += b;
    EXPECT_EQ(a.size(), 2);
    EXPECT_TRUE(a.contains(makeMove(2, 3)));
}

TEST(MoveListTest, OperatorPlusMove) {
    MoveList<256> ml;
    ml          += makeMove(0, 1);
    auto result  = ml + makeMove(2, 3);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(ml.size(), 1);  // original unchanged
}

TEST(MoveListTest, OperatorPlusList) {
    MoveList<256> a, b;
    a           += makeMove(0, 1);
    b           += makeMove(2, 3);
    auto result  = a + b;
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(a.size(), 1);  // original unchanged
}

TEST(MoveListTest, RangeFor) {
    MoveList<256> ml;
    ml        += makeMove(0, 1);
    ml        += makeMove(2, 3);
    int count  = 0;
    for (const Move& m : ml) count++;
    EXPECT_EQ(count, 2);
}

TEST(MoveListTest, Formatter) {
    MoveList<256> ml;
    ml += Move(Square(1, 4), Square(3, 4));
    ml += Move(Square(6, 3), Square(4, 3));
    EXPECT_EQ(fmt::format("{}", ml), "e2e4 d7d5");
}

TEST(MoveListTest, FormatterEmpty) {
    MoveList<256> ml;
    EXPECT_EQ(fmt::format("{}", ml), "");
}

TEST(MoveListTest, OutOfBoundsAccessAsserts) {
    MoveList<256> ml;
    EXPECT_DEATH(auto r = ml[0], "");
}

TEST(MoveListTest, PushBackOverCapacityAsserts) {
    MoveList<2> ml;
    ml.push_back(makeMove(0, 1));
    ml.push_back(makeMove(1, 2));
    EXPECT_DEATH(ml.push_back(makeMove(2, 3)), "");
}

TEST(MoveListTest, MergeOverCapacityAsserts) {
    MoveList<2> a, b;
    a += makeMove(0, 1);
    a += makeMove(1, 2);
    b += makeMove(2, 3);
    EXPECT_DEATH(a += b, "");
}