#include <iomanip>
#include <iostream>
#include <string>

#include <aleph/chess/board.hpp>

using namespace aleph::chess;

static uint64_t perft(Board board, int depth) {
    auto moves = board.getLegalMoves();
    if (depth == 0) return 1;
    uint64_t nodes = 0;
    for (const auto& move : moves) {
        nodes += perft(board.push(move), depth - 1);
    }
    return nodes;
}

static void dividePerft(const char* fen, int depth) {
    std::cout << "FEN: " << fen << "\n";
    std::cout << "Depth: " << depth << "\n";

    Board board(fen);
    auto moves = board.getLegalMoves();
    uint64_t total = 0;

    for (const auto& move : moves) {
        uint64_t nodes = perft(board.push(move), depth - 1);
        total += nodes;
        std::cout << std::left << std::setw(6) << move.toString()
                  << ": " << nodes << "\n";
    }

    std::cout << "\nTotal: " << total << std::endl;
}

int main() {
    dividePerft("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 2);
    return 0;
}