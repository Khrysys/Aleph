# Data Flow

This page describes how data moves through Aleph during inference and training. The two paths share some submodules but are otherwise distinct.

## Inference (UCI)

Inference is driven by the UCI protocol. The flow involves three concurrent loops.

### CLI loop

The CLI reads from `stdin` line by line, parsing UCI commands. On a `position` command, it constructs the starting `Board` via the chess submodule and applies any subsequent moves via `push()`. On a `go` command, it hands the current board to MCTS and starts the search. On `bestmove`, the result is written to `stdout`.

The CLI is also responsible for constructing the correct executor at startup — it instantiates the appropriate `ModelHandler` subclass (e.g. `aleph::executor::libtorch::ModelHandler`) and passes it to MCTS. From that point forward, MCTS programs against the `aleph::executor::ModelHandler` interface and has no knowledge of the backend.

### MCTS loop

MCTS owns the search tree and runs the main search loop. On each iteration it selects a leaf node using the search-contempt selection policy, packages the board position for evaluation, and routes it to the ModelHandler via the `executor-common` interface. Once the evaluation returns, the result is propagated back up the tree via backup. When the search budget is exhausted, the visit distribution at the root is used to select the best move, which is returned to the CLI.

Positions are routed to the ModelHandler by Zobrist hash modulo shard count. The eval cache is consulted before routing — a cache hit bypasses the ModelHandler entirely.

### ModelHandler loop

The ModelHandler runs on a dedicated thread per backend device (`cpu`, `cuda:0`, `cuda:1`, etc.). Multiple shards can map to the same device — the shard count is independent of the device count and exists to reduce write contention within the MCTS graph rather than to partition work across devices. The ModelHandler collects incoming position evaluation requests into batches, forwards the batch to the backend (via `aleph::executor::libtorch::ModelHandler` in the LibTorch case), and unpacks the results back into the per-position eval cache entries. The MCTS workers waiting on those cache entries are then unblocked.

> **Note:** The relationship between the MCTS graph, the transposition table, and the eval cache is currently being reworked. These are three distinct tables with different lifetimes and access patterns; their sharding and ownership model will be documented in the caching subsystem once that design is settled.

### Summary

```
stdin
  └── CLI (UCI parser)
        ├── chess (Board construction via push())
        └── MCTS (search loop)
              ├── executor::ModelHandler (interface, executor-common)
              │     └── executor::libtorch::ModelHandler (impl, executor-libtorch)
              │           └── LibTorch model (batch eval)
              └── caching (eval cache, Zobrist-keyed)
stdout
```

## Training

The training path is initiated by the CLI, which constructs the appropriate executor and hands it to `executor-training`. From that point, `executor-training` owns the training loop entirely.

`executor-training` is backend-agnostic — it depends only on `executor-common` and has no direct knowledge of LibTorch or any other backend. All backend-specific behavior is accessed through the `aleph::executor::ModelHandler` interface.

The training loop covers supervised pretraining and self-play. During supervised pretraining, preprocessed positions from the Lichess evaluated game database are fed directly into the training step. During self-play, the MCTS submodule generates games using the current best network, and the resulting positions are fed into the training step. Network promotion via SPRT is also managed within the training loop.

### Summary

```
CLI
  └── executor-training (training loop)
        ├── executor::ModelHandler (interface, executor-common)
        │     └── executor::libtorch::ModelHandler (impl, executor-libtorch)
        │           └── LibTorch model (forward + backward)
        ├── MCTS (self-play game generation)
        │     ├── chess (Board construction)
        │     └── caching (eval cache)
        └── SPRT (network promotion)
```