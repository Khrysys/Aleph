# Aleph (ℵ₀)

Aleph is a C++17/20 chess engine combining search-contempt Monte Carlo Tree Search (arXiv:2504.07757) with a cross-attention transformer evaluator. The evaluator uses a board self-attention encoder and a move encoder, bridged by cross-attention, to produce a policy over legal moves, a win/draw/loss value estimate, and a moves-left head. The entire stack — architecture, training loop, and self-play — is implemented in C++ with LibTorch; there is no Python training path.

```{toctree}
:maxdepth: 2
:caption: Contents

architecture/index
subsystems/index
```