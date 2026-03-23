# 0003 - Cross-Attention Transformer Evaluator

## Context

Aleph requires a neural network evaluator that produces a policy over legal moves, a win/draw/loss value estimate, and a moves-left prediction from a given board position. The evaluator architecture determines how the model represents the board, how it produces a policy, and how compute is distributed across those tasks.

### The dense policy problem

The conventional approach in engines derived from AlphaZero is a flat policy head over a fixed set of move encodings. AlphaZero uses 4672 output planes (8×8×73), encoding all possible from-square, to-square, and promotion combinations regardless of legality. Lc0, after much optimization, uses a more compact representation of 1858 elements — 56 planes for sliding moves across all directions and distances, 8 planes for knight moves, and 9 promotion planes valid only on the seventh rank — but the output size is still fixed and position-independent.

In both cases, illegal moves must be masked after the fact: the model produces logits over the full output space and a legality mask is applied before the softmax. This means the board embeddings must encode enough information to reconstruct the full policy distribution across the entire output space. This pushes required embedding sizes upward and couples the value and policy tasks inside the same representation, forcing the model to serve both purposes at once.

At large model scales this is a reasonable tradeoff — a linear projection from a large embedding dimension to 1858 outputs is negligible relative to the total parameter count. At the scale Aleph targets (10–12M parameters total), it is not. A single linear projection from E_board=256 to 1858 outputs costs 256 × 1858 = 475,648 parameters, plus 1858 bias terms — roughly 477K parameters, or approximately 4% of the entire model budget spent on a single output head before any board encoding has taken place. At E_board=512 that figure doubles to ~950K. This makes the dense policy head disproportionately expensive at small model sizes in a way that would be irrelevant at LLM scale, where decoder projections routinely cost hundreds of millions of parameters without dominating the budget.

An alternative is a small MLP applied independently to each legal move, conditioned on the board state. This decouples move count from output size but produces move scores without any interaction between moves, which limits the model's ability to reason about relative move quality.

### Cross-attention as a natural fit

Cross-attention between a board representation and a set of move embeddings addresses both problems directly. The board self-attention stack operates over the 64 squares and produces refined, pattern-aware square representations without any responsibility for the policy. The move embeddings — one per legal move — attend to those refined square representations via cross-attention. Because the moves are the inputs to the policy head directly, the policy is naturally sparse: only legal moves exist as queries, so there is no probability mass to suppress over illegal moves and no need for the board to encode them.

The ordering matters. If cross-attention were applied before any board self-attention, the move queries would be attending to raw, uncontextualized square embeddings that have had no opportunity to form relationships with other squares. The cross-attention layer would then be doing the work of a full self-attention stack on its own, with the moves forced to learn the entire board from scratch. Applying board self-attention first means the moves are attending to embeddings that have already connected across the board — piece coordination, pawn structure, king safety, and other positional patterns are already partially encoded. The cross-attention can then focus on extracting move-relevant information from a representation that is already meaningful.

This also allows the board self-attention stack to focus almost entirely on value. The policy is handled by the moves themselves, which are the policy map. The board representation can optimize for positional understanding without being pulled toward policy concerns, and the policy head serves as a form of pseudo-distillation off the value representation.

## Decision

The evaluator is a cross-attention transformer with the following structure:

- **8 board self-attention layers** over the 64 squares, with embedding dimension E_board=256, 32 heads at 8 dimensions per head.
- **6 cross-attention layers** where legal move embeddings attend to the board representation.
- **1 post-cross move self-attention layer** over the legal moves after cross-attention.

Output heads operate over the final move representations for policy and over a pooled board representation for value and moves-left.

### Move embedding

Each legal move is embedded by concatenating three sub-embeddings — from-square, to-square, and promotion — each of dimension 64, and projecting through a linear layer from dimension 192 to E_move=128. The promotion embedding has 10 entries: 0=none, 1=white knight, 2=white bishop, 3=white rook, 4=white queen, 5=black knight, 6=black bishop, 7=black rook, 8=black queen. Pawns and kings are excluded as promotion targets and the indices are shifted down accordingly.

Summation was considered but rejected: each sub-embedding is semantically incomplete without the others. A from-square embedding carries no meaning without knowing where the piece is going or whether it promotes. Concatenation followed by a learned linear projection preserves the full joint information and allows the model to learn how the three components interact, rather than assuming they combine additively.

### Post-cross move self-attention

After cross-attention, a single move self-attention layer allows moves to compare themselves against each other. This addresses cases where multiple moves appear superficially similar after attending to the board — for instance, two rook moves to adjacent squares — and allows the model to self-order them before producing policy logits. This layer is tentative. The throughput cost of an additional attention layer may exceed the Elo benefit it provides, and it may be removed in a future revision pending empirical results.

### Layer counts

The layer counts — 8 board self-attention, 6 cross-attention, 1 post-cross move self-attention — are starting points derived by targeting a model approximately one tenth the size of Lc0-BT4. They are expected to be revised as training results accumulate. The board self-attention stack is considered more important than the cross-attention stack, as the quality of the board representation underpins both the value estimate and the policy via the cross-attention readout.

## Consequences

**Positive**

- The policy head is naturally sparse over legal moves only. No probability mass suppression over illegal moves is needed, and the output size scales with the number of legal moves rather than being fixed at 4096.
- The board embedding can focus on value without being pulled toward encoding policy information, potentially allowing a smaller board embedding to reach the same representational quality as a larger dense-policy board embedding.
- Move embeddings carry full joint information about the from-square, to-square, and piece via concatenation, rather than assuming additive decomposition.
- The architecture is well-motivated by the structure of chess: the board encodes position, and the moves attend to that position to produce a policy.

**Negative**

- Cross-attention over a variable-length set of legal moves complicates batching. Positions have different numbers of legal moves, so move tensors must be padded or packed across a batch, adding implementation complexity relative to a fixed-size output head.
- The architecture is novel in this specific combination. There is no directly comparable engine to validate against, and empirical tuning will be required to determine whether the layer counts, embedding sizes, and structural choices are well-calibrated.
- The post-cross move self-attention layer is of uncertain value and may represent wasted compute. It is retained for now but its removal should be revisited once training results are available.
- Smolgen on both board self-attention and cross-attention adds implementation complexity, particularly for cross-attention where the per-move formulation must handle variable move counts.