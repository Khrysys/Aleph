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
    std::cout << "Depth: " << depth << "\n" << std::endl;

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
    dividePerft("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 5);
    dividePerft("r3k2r/Pppp1ppp/1b3nbN/nPP5/BB2P3/q4N2/Pp1P2PP/R2Q1RK1 b kq - 0 1", 4);
    //dividePerft("8/2p5/3p4/KP5r/7k/5p2/4P1P1/1R6 w - - 0 2", 3);
    //dividePerft("8/2p5/3p4/1P5r/1K5k/5p2/4P1P1/1R6 b - - 1 2", 2);
    return 0;
}