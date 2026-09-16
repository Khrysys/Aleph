# Aleph::Chess Overview

Aleph::Chess is a highly specialized library, having taken reference from Stockfish's chess library and also [Disservin's Chess Library](https://github.com/disservin/chess-library), mostly for correctness. At the expense of speed, and to avoid getting too in the weeds of optimizing the implementation, it is around 50-70% as fast as Disservin's implementation. Notably, it is much slower when it comes to heavy branching, where moves are made and popped from the stack often, because in Aleph::Chess, that means creating a new 128-byte object and throwing it away frequently. Perhaps some optimization could be done to prevent this particular case of alloc/dealloc, but I believe this to be only to improve benchmarks and would not reasonably improve performance in the engine. 

## Internal Structure

### Attack Tables

Aleph uses generated attack tables to produce moves. This is done with a relatively small Python script contained in the `tools` directory. This ouputs a .inl file into the build directory, which can then be picked up by the main CMake scripts and added to the header list for Aleph::Chess when installing or packaging so it doesn't get left behind. 

# Board

The board is a fixed size, 128-byte object with no move stack. This means there is no automatic threefold repetition, but there is the 50 move rule. The reason for this is that the move stack would have made each board object considerably larger and would have required more complicated logic for doing and undoing moves, whereas without it, we were just above one 64-byte cache line with no way to go below, and so to improve speed, mutable fields for caching critical information and the 50 move rule were added. For the most part, I decided that the engine should trust the neural network about what moves it wants to make. If it decides that the best move truly is to repeat or that it can't find a way through in 50 moves, that means training the network better, rather than adding in knowledge of the move stack.