# 0004 - FFN Elimination and Pre/Post Projection

## Context

Standard transformer blocks consist of two sub-layers: a multi-head self-attention layer and a position-wise feed-forward network (FFN). The FFN is typically two linear projections with a nonlinearity between them, expanded to 4x the embedding dimension in the intermediate layer. This 4x expansion is a near-universal default in transformer literature originating from the original "Attention is All You Need" architecture, and is retained in most subsequent work without domain-specific justification.

### Lc0's findings on FFN width in chess transformers

The Leela Chess Zero team conducted systematic experiments on transformer architecture for chess, documented at https://lczero.org/blog/2024/02/transformer-progress/. Their findings showed that the FFN contributes negligible benefit in chess transformers compared to its compute cost. Reducing the FFN from the standard 4x intermediate width down to 1x — a single linear layer with no expansion — produced no meaningful loss in playing strength while significantly reducing parameter count and inference cost. This result is specific to chess: the positional structure of the game may mean that the mixing and nonlinearity provided by a wide FFN is largely redundant given sufficient attention depth.

### Taking the conclusion further

If a 1x FFN provides the same benefit as a 4x FFN, the natural question is whether the FFN is providing any benefit at all beyond nonlinearity and linear mixing — both of which can be provided in other ways. Aleph takes Lc0's finding to its conclusion and eliminates the FFN entirely from all attention layers. The nonlinearity and mixing that the FFN would have provided are instead supplied by pre and post projections around the attention computation itself, activated with SiLU.

## Decision

All attention layers in Aleph — board self-attention, cross-attention, and move self-attention — use the following pattern in place of a standard attention block with FFN:

1. A **pre-projection** maps the input to the projection dimension P=128 and applies SiLU. This replaces the role of the FFN's first linear layer.
2. QKV projections operate in projection space rather than in the full embedding dimension.
3. Scaled dot-product attention is computed over the projected QKV representations.
4. A **post-projection** maps the attention output back from the concatenated head dimension and applies SiLU. This replaces the role of the FFN's second linear layer.
5. A final **output projection** maps to the target embedding dimension.
6. A residual connection and post-norm are applied over the original input and the output projection result.

There is no separate FFN sub-layer. The nonlinearity is injected at the pre and post projections rather than in a dedicated feed-forward block.

### Board self-attention

```cpp
torch::Tensor BoardSelfAttentionImpl::forward(torch::Tensor boards) {
    auto projectedBoards = torch::silu(preProjection(boards));                                                      // [B, 64, P]
    auto key = keyProjection(projectedBoards).view({-1, 64, _numHeads, _headDim}).permute({0, 2, 1, 3});            // [B, H, 64, D]
    auto query = queryProjection(projectedBoards).view({-1, 64, _numHeads, _headDim}).permute({0, 2, 1, 3});        // [B, H, 64, D]
    auto value = valueProjection(projectedBoards).view({-1, 64, _numHeads, _headDim}).permute({0, 2, 1, 3});        // [B, H, 64, D]
    auto smolgen = smolgenGenerator(projectedBoards);                                                               // [B, H, 64, 64]

    auto attentionScores = at::scaled_dot_product_attention(query, key, value, smolgen);                            // [B, H, 64, D]
    auto updates = attentionScores.permute({0, 2, 1, 3}).contiguous().view({-1, 64, _numHeads * _headDim});         // [B, 64, H*D]
    auto projectedOutput = torch::silu(postProjection(updates));                                                    // [B, 64, P]
    auto output = outputProjection(projectedOutput);                                                                // [B, 64, E_board]
    return postNorm(boards + output);
}
```

### Cross-attention

Cross-attention follows the same pattern with one structural difference: boards and moves receive separate pre-projections. Queries are derived from the projected move representation; keys and values are derived from the projected board representation. The residual connection is over the move input, not the board input.

```cpp
torch::Tensor CrossAttentionImpl::forward(torch::Tensor boards, torch::Tensor moves, torch::Tensor moveMask) {
    auto projectedBoards = torch::silu(boardPreProjection(boards));                                                 // [B, 64, P]
    auto projectedMoves = torch::silu(movePreProjection(moves));                                                    // [B, M, P]
    auto M = moves.size(1);
    auto query = queryProjection(projectedMoves).view({-1, M, _numHeads, _headDim}).permute({0, 2, 1, 3});          // [B, H, M, D]
    auto key = keyProjection(projectedBoards).view({-1, 64, _numHeads, _headDim}).permute({0, 2, 1, 3});            // [B, H, 64, D]
    auto value = valueProjection(projectedBoards).view({-1, 64, _numHeads, _headDim}).permute({0, 2, 1, 3});        // [B, H, 64, D]

    auto attentionScores = at::scaled_dot_product_attention(query, key, value);                                     // [B, H, M, D]
    auto updates = attentionScores.permute({0, 2, 1, 3}).contiguous().view({-1, M, _numHeads * _headDim});          // [B, M, H*D]
    auto projectedOutput = torch::silu(postProjection(updates));                                                    // [B, M, P]
    auto output = outputProjection(projectedOutput);                                                                // [B, M, E_move]
    return postNorm(moves + output);
}
```

The move mask is generated prior to the model forward pass and applied at the attention score level.

## Consequences

**Positive**

- The FFN — which Lc0's research demonstrated provides negligible benefit in chess transformers — is eliminated entirely, reducing parameter count and inference cost without expected loss in playing strength.
- Nonlinearity is preserved through SiLU activations on the pre and post projections. The model retains the ability to learn non-linear functions without a dedicated FFN sub-layer.
- QKV projections operate in projection dimension P=128 rather than the full embedding dimension E_board=256, reducing the cost of the QKV projections themselves.
- The pattern is consistent across all three attention layer types (board self-attention, cross-attention, move self-attention), keeping the architecture uniform and reducing the surface area for implementation mistakes.

**Negative**

- This is a departure from standard transformer architecture. The pre/post projection pattern is not widely validated in the literature, and there is limited prior work to draw on for troubleshooting training instabilities or unexpected behavior.
- The Lc0 findings that motivate FFN elimination were established on their specific architecture and training setup. It is possible that the interaction between FFN elimination and Aleph's cross-attention structure produces different results than expected.
- Eliminating the FFN removes a well-understood mechanism for position-wise mixing. If the pre/post projections prove insufficient as a substitute, restoring the FFN would require architectural changes and retraining from scratch.