# 0001 - Immutable Board with Copy-on-Write via push()

## Context

Aleph requires a chess board representation that is safe and efficient to use across many concurrent MCTS worker threads. Each worker needs to speculatively apply moves, evaluate positions, and then discard that line of play and return to the root — repeatedly, for the duration of a search.

Three approaches were considered:

**Use an existing library as-is.** Stockfish's position code is among the most battle-tested chess representations in existence, but it is so deeply involved with the rest of Stockfish's infrastructure that disentangling it into a standalone library would require more effort than writing one from scratch. Disservin's chess library (`chess-library`) is a well-regarded standalone option and among the fastest available, but carries design choices that serve CPU evaluators and general-purpose tooling rather than a neural network engine. Most notably, its `Move` type is 32-bit and bundles a score field alongside the from/to squares — useful in some contexts, unnecessary overhead here. It also uses a mutable board with an undo mechanism: to reverse a move, the caller must pass back the exact same undo object that was produced when the move was applied. Disservin's own documentation notes this requirement explicitly. This approach is fragile, easy to misuse, and unpleasant to reason about in a multithreaded context where workers are constantly branching and backtracking.

**Mutable board with an undo stack.** Implementing a custom mutable board with an undo mechanism avoids the dependency but inherits the same fragility. Maintaining correct undo state across a deep search tree is a known source of bugs, and the complexity cost is ongoing — every new move type or board feature must be correctly mirrored in the undo path.

**Immutable board with copy-on-write.** `push()` returns a new `Board` instance with the move applied, leaving the original untouched. A worker that needs to explore a line copies the current board, applies moves freely, and discards the copy when done. There is no undo path to maintain and no shared mutable state between workers.

## Decision

Aleph uses an immutable `Board` type. `push()` returns a new `Board` by value. Workers hold a local copy of the board for their subtree and discard it when backtracking.

The `Move` type is 16-bit, encoding only from-square, to-square, and promotion piece. Castling and en passant are detected contextually inside `push()` rather than encoded in the move itself.

## Consequences

**Positive**

- No undo mechanism is needed anywhere in the codebase.
- Workers can freely apply moves without coordinating with other threads.
- Board copies are cheap relative to the cost of neural network evaluation, which dominates search time by a large margin. The copy cost is not a bottleneck.
- The chess library can be header-only and `constexpr`-pervasive without having to manage mutable rollback state.

**Negative**

- The library is noticeably less flexible than general-purpose alternatives. The absence of an undo mechanism and the stripped-down `Move` type make it a poor fit for applications outside of Aleph's specific use case — a CPU evaluator, a perft harness, or any tool that expects a conventional mutable board interface would need significant additions or a different library entirely.
- Every call to `push()` allocates a new `Board` on the stack. For the vast majority of search nodes this is fine, but deep lines produce a chain of board copies that a mutable design would avoid entirely. This is unlikely to be a practical bottleneck given NN eval cost, but it is a real difference in memory behavior.
- Repetition detection requires the caller to maintain an external history of Zobrist hashes. A mutable board with an undo stack naturally accumulates this history as a side effect; an immutable board does not.
- The 16-bit `Move` type and contextual castling/en passant detection in `push()` mean that moves are not self-describing. A move cannot be interpreted correctly without a board context, which complicates any use case that needs to serialize, log, or transmit moves independently of board state.