# 0005 - Smolgen for Board Self-Attention

## Context

Standard multi-head attention uses either no positional bias or a fixed learned positional bias added to the attention logits before softmax. For chess, the relationship between squares is highly structured — files, ranks, diagonals, and piece-specific movement patterns all create strong positional priors. A fixed learned bias can capture these relationships but does not vary with the input: it is the same regardless of what pieces are on the board.

Smolgen, introduced by the Leela Chess Zero team and documented at https://lczero.org/blog/2024/02/transformer-progress/, replaces the fixed positional bias with a generated one derived from the input itself. Rather than learning a static [H, 64, 64] bias matrix, smolgen generates a position-dependent attention bias from the current board state. This allows the attention pattern to adapt to the specific position being evaluated rather than applying a fixed prior. Lc0's experiments showed significant improvement from smolgen, motivating its inclusion in Aleph's board self-attention layers.

## Decision

### Board self-attention smolgen

Aleph's smolgen implementation follows Lc0's formulation directly. It takes the pre-projected board tensor (post-SiLU, dimension E_proj=128) as input and generates a [B, H, 64, 64] attention bias through a four-stage pipeline:

1. **Square projection** — a per-square linear from P to S_S (smolgen square dimension) compresses each square's representation independently.
2. **Board projection** — the 64 square representations are flattened to a single vector of dimension 64×S_S and projected down to S_B (smolgen board dimension), producing a single board-level summary vector.
3. **Head projection** — the board vector is projected to H×S_B and reshaped to [B, H, S_B], producing a per-head board summary.
4. **Output projection** — each head's S_B-dimensional vector is projected to 64×64 and reshaped to [B, H, 64, 64], producing the final attention bias.

```cpp
torch::Tensor SmolgenGeneratorImpl::forward(torch::Tensor projBoards) {
    auto squareSmolgen = smolgenSquareProjection(projBoards).view({-1, 64 * _smolgenSquareDim});    // [B, 64 * S_S]
    auto boardSmolgen = smolgenBoardProjection(squareSmolgen);                                      // [B, S_B]
    auto headSmolgen = smolgenHeadProjection(boardSmolgen).view({-1, _numHeads, _smolgenBoardDim}); // [B, H, S_B]
    auto smolgen = smolgenOutputProjection(headSmolgen).view({-1, _numHeads, 64, 64});              // [B, H, 64, 64]
    return smolgen;
}
```

All smolgen linear layers have no bias, consistent with the rest of Aleph's internal linear layers.

### Cross-attention smolgen

Cross-attention requires a bias of shape [B, H, M, 64] — one row per legal move attending to each of the 64 squares. This makes the standard smolgen formulation inapplicable: smolgen compresses the board to a fixed-size vector and expands back to [H, 64, 64], but the move dimension M is variable and cannot be recovered from a board-only compression.

The naive extension — compressing the board to a fixed vector and using an einsum to introduce M — produces a bias whose cost is functionally equivalent to an additional cross-attention head while providing less expressivity than one. This was rejected.

**Current status:** cross-attention smolgen is not implemented. The cross-attention layers use no positional bias.

**Candidate formulation under consideration:** rather than compressing the board and expanding to M, compress the moves and expand to 64. The pipeline would be:

1. Pool move embeddings to a global move context vector (mean pool or learned aggregation over M): [B, M, E_M] → [B, E_M]
2. Broadcast the resulting global vector to each move, SiLU, linear mix, multiply elementwise to moves: [B, E_M] -> [B, M, E_M]
3. Project each conditioned move embedding to H×64 and reshape to [B, H, M, 64]

This keeps cost at O(M) and gives each move a distinct bias conditioned on both its own embedding and the global move distribution. It avoids any M² terms and does not require a fixed move count. This formulation has not yet been validated empirically and is not yet implemented.

## Consequences

**Positive**

- Board self-attention generates input-dependent attention biases, allowing attention patterns to adapt to the specific position rather than applying a fixed prior. Lc0's results indicate this is a meaningful improvement.
- The smolgen pipeline is entirely linear and cheap relative to the attention computation itself.
- All smolgen layers are bias-free, consistent with the rest of the architecture.

**Negative**

- Cross-attention currently has no positional bias. This is a known gap — moves attending to board squares have no explicit guidance about positional relationships between the move's origin/destination and the squares being attended to.
- The candidate cross-attention smolgen formulation compresses moves into a global context vector before reintroducing per-move variation. The global pooling step may lose move-specific information that would be useful for generating per-move biases. Whether this loss is acceptable in practice is unknown.
- Smolgen adds two hyperparameters (S_S and S_B) that require tuning. Their interaction with E_proj and the number of heads is not well-characterized outside of Lc0's specific architecture.