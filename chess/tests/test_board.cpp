#include <gtest/gtest.h>
#include <aleph/chess/board.hpp>

using namespace aleph::chess;

// --- Valid positions ---

TEST(BoardFenTest, DefaultConstructor) {
    EXPECT_NO_THROW(Board());
}

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

// --- Field count ---

TEST(BoardFenTest, TooFewFields) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq"), std::invalid_argument);
}

// --- Piece placement ---

TEST(BoardFenTest, TooManyRanks) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), std::invalid_argument);
}

TEST(BoardFenTest, TooFewRanks) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), std::invalid_argument);
}

TEST(BoardFenTest, RankExceeds8Squares) {
    EXPECT_THROW(Board("rnbqkbnrr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), std::invalid_argument);
}

TEST(BoardFenTest, RankUnder8Squares) {
    EXPECT_THROW(Board("rnbqkbn/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), std::invalid_argument);
}

TEST(BoardFenTest, InvalidPieceCharacter) {
    EXPECT_THROW(Board("rnbqkbnx/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), std::invalid_argument);
}

// --- King validation ---

TEST(BoardFenTest, MissingWhiteKing) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQQBNR w KQkq - 0 1"), std::invalid_argument);
}

TEST(BoardFenTest, MissingBlackKing) {
    EXPECT_THROW(Board("rnbqqbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), std::invalid_argument);
}

TEST(BoardFenTest, TwoWhiteKings) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKKNR w KQkq - 0 1"), std::invalid_argument);
}

// --- Pawn validation ---

TEST(BoardFenTest, PawnOnRank1) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNP w KQkq - 0 1"), std::invalid_argument);
}

TEST(BoardFenTest, PawnOnRank8) {
    EXPECT_THROW(Board("rnbqkbnP/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), std::invalid_argument);
}

TEST(BoardFenTest, TooManyWhitePawns) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/PPPPPPPPP/8/RNBQKBNR w KQkq - 0 1"), std::invalid_argument);
}

// --- Castling validation ---

TEST(BoardFenTest, CastlingWhiteKingsideNoRook) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBN1 w K - 0 1"), std::invalid_argument);
}

TEST(BoardFenTest, CastlingWhiteKingsideNoKing) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQ1BNR w K - 0 1"), std::invalid_argument);
}

TEST(BoardFenTest, CastlingBlackQueensideNoRook) {
    EXPECT_THROW(Board("1nbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w q - 0 1"), std::invalid_argument);
}

TEST(BoardFenTest, InvalidCastlingCharacter) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w X - 0 1"), std::invalid_argument);
}

// --- En passant validation ---

TEST(BoardFenTest, EnPassantWrongRankForBlackToMove) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e6 0 1"), std::invalid_argument);
}

TEST(BoardFenTest, EnPassantWrongRankForWhiteToMove) {
    EXPECT_THROW(Board("rnbqkbnr/pppp1ppp/8/4p3/8/8/PPPPPPPP/RNBQKBNR w KQkq e3 0 1"), std::invalid_argument);
}

TEST(BoardFenTest, EnPassantNoPawnPresent) {
    EXPECT_THROW(Board("rnbqkbnr/pppp1ppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq e3 0 1"), std::invalid_argument);
}

TEST(BoardFenTest, EnPassantInvalidFile) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq x3 0 1"), std::invalid_argument);
}

// --- Halfmove clock ---

TEST(BoardFenTest, HalfmoveClockValid) {
    EXPECT_NO_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 42 1"));
}

TEST(BoardFenTest, HalfmoveClockExceeds100) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 101 1"), std::invalid_argument);
}

TEST(BoardFenTest, HalfmoveClockInvalidCharacter) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - x 1"), std::invalid_argument);
}

// --- Fullmove number ---

TEST(BoardFenTest, FullmoveInvalidCharacter) {
    EXPECT_THROW(Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 x"), std::invalid_argument);
}

// --- push() tests ---

// Helper to make a move from algebraic notation
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

// --- Side to move ---

TEST(BoardPushTest, FlipsSideToMove) {
    Board b;
    EXPECT_FALSE(b.isBlackTurn());
    Board b2 = b.push(makeAlgebraicMove("e2", "e4"));
    EXPECT_TRUE(b2.isBlackTurn());
    Board b3 = b2.push(makeAlgebraicMove("e7", "e5"));
    EXPECT_FALSE(b3.isBlackTurn());
}

// --- Pawn moves ---

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

// --- En passant ---

TEST(BoardPushTest, WhiteDoublePushSetsEnPassant) {
    Board b;
    Board b2 = b.push(makeAlgebraicMove("e2", "e4"));
    EXPECT_TRUE(b2.isEnPassantValid());
    EXPECT_EQ(b2.enPassantFile(), 4); // e-file
}

TEST(BoardPushTest, BlackDoublePushSetsEnPassant) {
    Board b("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1");
    Board b2 = b.push(makeAlgebraicMove("d7", "d5"));
    EXPECT_TRUE(b2.isEnPassantValid());
    EXPECT_EQ(b2.enPassantFile(), 3); // d-file
}

TEST(BoardPushTest, NonDoublePushClearsEnPassant) {
    Board b;
    Board b2 = b.push(makeAlgebraicMove("e2", "e4"));
    EXPECT_TRUE(b2.isEnPassantValid());
    Board b3 = b2.push(makeAlgebraicMove("e7", "e6"));
    EXPECT_FALSE(b3.isEnPassantValid());
}

TEST(BoardPushTest, WhiteEnPassantCapture) {
    // e5, black plays d5, white captures en passant exd6
    Board b("rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 1");
    Board b2 = b.push(makeAlgebraicMove("e5", "d6"));
    EXPECT_EQ(b2.get(Square(5, 3)).type(), PAWN);  // white pawn on d6
    EXPECT_EQ(b2.get(Square(4, 3)).type(), NONE);  // captured pawn removed from d5
    EXPECT_EQ(b2.get(Square(4, 4)).type(), NONE);  // e5 vacated
}

TEST(BoardPushTest, BlackEnPassantCapture) {
    Board b("rnbqkbnr/ppp1pppp/8/8/3pP3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    Board b2 = b.push(makeAlgebraicMove("d4", "e3"));
    EXPECT_EQ(b2.get(Square(2, 4)).type(), PAWN);  // black pawn on e3
    EXPECT_EQ(b2.get(Square(3, 4)).type(), NONE);  // captured pawn removed from e4
    EXPECT_EQ(b2.get(Square(3, 3)).type(), NONE);  // d4 vacated
}

// --- Captures ---

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

// --- Promotions ---

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

// --- Castling moves ---

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

// --- Castling rights ---

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

// --- Halfmove clock ---

TEST(BoardPushTest, HalfmoveClockIncrements) {
    Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    Board b2 = b.push(makeAlgebraicMove("g1", "f3"));
    EXPECT_EQ(b2.halfMoveClock(), 1);
}

TEST(BoardPushTest, HalfmoveClockResetsOnPawnMove) {
    Board b("rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R w KQkq - 5 1");
    Board b2 = b.push(makeAlgebraicMove("e2", "e4"));
    EXPECT_EQ(b2.halfMoveClock(), 0);
}

TEST(BoardPushTest, HalfmoveClockResetsOnCapture) {
    Board b("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 5 1");
    Board b2 = b.push(makeAlgebraicMove("e4", "d5"));
    EXPECT_EQ(b2.halfMoveClock(), 0);
}

// --- Occupancy ---

TEST(BoardPushTest, OccupancyUpdatesAfterMove) {
    Board b;
    Board b2 = b.push(makeAlgebraicMove("e2", "e4"));
    EXPECT_TRUE(b2.occupancy() & (1ULL << static_cast<uint8_t>(Square(3, 4))));
    EXPECT_FALSE(b2.occupancy() & (1ULL << static_cast<uint8_t>(Square(1, 4))));
}

TEST(BoardPushTest, OccupancyUpdatesAfterCapture) {
    Board b("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1");
    Board b2 = b.push(makeAlgebraicMove("e4", "d5"));
    uint64_t occ = b2.occupancy();
    EXPECT_TRUE(occ  & (1ULL << static_cast<uint8_t>(Square(4, 3))));
    EXPECT_FALSE(occ & (1ULL << static_cast<uint8_t>(Square(3, 4))));
}

// --- Immutability ---

TEST(BoardPushTest, OriginalBoardUnchangedAfterPush) {
    Board b;
    uint64_t occBefore = b.occupancy();
    auto r = b.push(makeAlgebraicMove("e2", "e4"));
    EXPECT_EQ(b.occupancy(), occBefore);
}

// --- Piece identity after move ---

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