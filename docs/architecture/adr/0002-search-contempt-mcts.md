# 0002 - Search-Contempt MCTS over Standard PUCT

## Context

Aleph requires a search algorithm that drives self-play training. The training signal quality — specifically, the diversity and difficulty of positions generated during self-play — has a direct impact on how efficiently the network improves per game played. Two candidates were seriously considered.

**Alpha-Beta search** was discounted early. AB operates sequentially and relies on CPU-friendly shallow evaluations; it does not parallelize well across GPU batch inference and would leave Aleph's hardware severely underutilized. It is not a realistic option for a GPU-first neural engine.

**Standard PUCT MCTS** (as used by AlphaZero and Leela Chess Zero) selects moves at every node using the PUCT formula, which balances exploitation of high-value moves with exploration of under-visited ones. Both players are treated symmetrically — the search models the opponent as playing optimally in the same way it models itself. The result is that self-play games tend toward balanced, drawish positions as the network improves, because both sides are trying to win by the same strategy. This narrows the distribution of training positions and reduces the signal available to the network, particularly in positions where the engine needs to learn to fight or defend under pressure. Historically, reaching strong play with pure PUCT self-play has required tens of millions of training games.

**Search-contempt MCTS** (Joshi, arXiv:2504.07757) breaks the symmetry between the two players during self-play. The player node always uses PUCT. The opponent node uses PUCT below a visit threshold N_scl, then switches to Thompson Sampling over a frozen snapshot of its visit distribution once that threshold is crossed. Thompson Sampling draws a move randomly from the frozen distribution rather than greedily exploiting it, which causes the opponent to play a wider variety of moves — including weaker ones — in proportion to how often those moves were visited. This forces the player into more challenging, unbalanced positions and produces a richer training distribution. The paper demonstrates that this approach can achieve competitive strength with training game counts in the hundreds of thousands rather than tens of millions.

## Decision

Aleph uses search-contempt MCTS. Player nodes always use PUCT. Opponent nodes use PUCT below N_scl visits, then Thompson Sampling over a frozen visit distribution snapshot above N_scl. The tree is cleared after every move, as required by the algorithm. This is not optional — retaining the tree across moves would invalidate the frozen snapshot mechanism.

## Hyperparameters

**N_scl** is the visit threshold at which an opponent node transitions from PUCT to Thompson Sampling. It is the primary hyperparameter of search-contempt.

- Low N_scl causes the transition to happen early, when the visit distribution is noisy and undersampled. Thompson Sampling over a poor distribution produces near-random opponent moves, which degrades the training signal rather than improving it.
- High N_scl delays the transition, causing the opponent to behave more like standard PUCT for most of the search budget. The contempt effect diminishes and the training distribution converges toward standard self-play.
- N_scl=5 is the empirically optimal value for early training, when the win+loss to draw ratio (w+l/d) is approximately 1. As the network improves and draws become more frequent, N_scl should be increased according to the schedule described in Figure 6 of the paper.

**c_puct** is the exploration constant in the PUCT formula, shared between player and opponent nodes during the PUCT phase. It controls the balance between visiting high-prior moves and visiting under-explored ones. This is not specific to search-contempt; its tuning follows standard PUCT practice.

## Consequences

**Positive**

- Self-play generates a richer, more varied distribution of positions. The network is exposed to unbalanced positions it must learn to navigate, which accelerates improvement.
- Competitive strength is theoretically achievable with significantly fewer training games than standard AlphaZero-style self-play, reducing compute cost.
- The algorithm also improves strength in odds chess (asymmetric starting positions), which may be useful for future testing or training curriculum design.

**Negative**

- The tree must be cleared after every move without exception. This eliminates tree reuse, which is a meaningful efficiency optimization in standard MCTS engines. Aleph cannot benefit from accumulated visit counts across moves.
- N_scl requires active management. The optimal value is not fixed; it must be scheduled as a function of the network's draw rate. An incorrect schedule — particularly one that fails to increase N_scl as the network matures — will degrade training efficiency.
- The asymmetry between player and opponent introduces a parameter (N_scl) that standard PUCT does not have. This adds a tuning surface that must be monitored throughout training.
- The algorithm is newer and less battle-tested than standard PUCT. The empirical results in the paper are promising but the approach has not yet seen the breadth of validation that PUCT has accumulated across years of production engine development.