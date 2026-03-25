# Architecture Decision Records

An Architecture Decision Record (ADR) documents a significant design decision: the context that motivated it, the options considered, and the rationale for the choice made. ADRs are not prescriptive — they exist to make it possible to retrace the logic behind how Aleph is built, and to avoid relitigating settled questions without new information.

ADR files are named `NNNN-short-title.md`, where `NNNN` is a zero-padded four-digit sequence number. Numbers are assigned in the order decisions are recorded, not in any order of importance. Once written, an ADR is not edited to reverse its conclusion — if a decision is revisited, a new ADR is written that references the original.

```{toctree}
:maxdepth: 1
:caption: Records

0001-immutable-chess-objects
0002-search-contempt-mcts
0003-cross-attention-transformer
0004-feed-forward-network-removal
0005-smolgen
0006-activation-function
0007-sprt-network-promotion
0008-supervised-pretraining.md
```