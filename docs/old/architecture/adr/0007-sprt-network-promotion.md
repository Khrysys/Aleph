# 0007 - SPRT for Network Promotion

## Context

During self-play training, a candidate network must be evaluated against the current best network to determine whether it should be promoted. The naive approach is to play a fixed number of games and promote if the win rate exceeds some threshold. This has two problems. First, a fixed game count is wasteful when the result is already clear early — a much stronger or much weaker candidate is identifiable well before the full game count is reached. Second, early in training the network improves rapidly; minimizing the number of games needed per promotion decision directly reduces the total compute cost of the training loop.

The Sequential Probability Ratio Test (SPRT) addresses both problems. It is a sequential hypothesis test that accumulates evidence game by game and stops as soon as the evidence is strong enough to accept one of the hypotheses. The expected number of games to reach a decision is much lower than a fixed game count when the true difference is large, and only approaches the fixed game count when the networks are genuinely close in strength.

The standard two-hypothesis SPRT used by Lc0 tests H0 (candidate is not stronger) against H1 (candidate is stronger). A candidate that is statistically equivalent to the current best and one that is clearly weaker both result in rejection, with no distinction between the two outcomes. This matters operationally: when a candidate is rejected because it is genuinely weaker, training should continue generating data and produce a new candidate. When a candidate is rejected because it is statistically equivalent — neither better nor worse — the correct response is to reset the test statistics and continue accumulating games with the same candidate rather than discarding it and starting over. A two-hypothesis test cannot distinguish these cases. A three-hypothesis test can, and the equivalence result (H0) serves as an explicit signal to reset and continue rather than reject and discard.

## Decision

Network promotion uses a three-hypothesis SPRT. The test runs game by game and stops as soon as any one of the following hypotheses crosses its confidence threshold:

- **H0 (equivalence):** the candidate and current best network are within 10 Elo of each other. Accepted at 99% confidence. The candidate is not promoted; the current best is retained.
- **H1 (candidate stronger):** the candidate network is stronger than the current best by at least 10 Elo. Accepted at 95% confidence. The candidate is promoted to become the new best network.
- **H2 (candidate weaker):** the candidate network is weaker than the current best by at least 10 Elo. Accepted at 95% confidence. The candidate is rejected; the current best is retained.

The 10 Elo threshold corresponds to approximately 1–2% more wins at equal draw rates, which is a meaningful but achievable improvement signal early in training. AlphaZero used approximately a 20 Elo threshold (53% win rate); 10 Elo is used here to be more sensitive to smaller improvements during the early phase of training when the network is improving rapidly.

The false positive rate (promoting a weaker network) and false negative rate (missing a stronger network) are bounded by the confidence thresholds: α=0.05 for H1 and H2, α=0.01 for H0. These are standard values used in engine testing communities including Lc0 and Stockfish's testing framework.

Games are played with paired openings — each opening position is played twice, once with the candidate as white and once as black. This reduces variance from opening bias without requiring a larger game count to reach the same confidence level.

## Consequences

**Positive**

- The expected game count per promotion decision is minimized when the true difference is large, which is exactly the regime that dominates early in training.
- The three-hypothesis formulation distinguishes between a statistically equivalent candidate (reset and continue) and a clearly weaker one (reject and discard), which is the operationally correct response in each case. A two-hypothesis test treats both as rejection and discards candidates that may simply need more training games to demonstrate improvement.
- The confidence thresholds provide a principled bound on promotion errors rather than relying on an arbitrary win rate cutoff.
- Paired openings reduce variance without increasing game count.

**Negative**

- The 10 Elo threshold is a judgment call. If the network's improvement per training cycle is consistently below 10 Elo, H0 will be accepted frequently and training will proceed without promotion even when the candidate is genuinely marginally better. The threshold may need to be revisited as training matures.
- SPRT assumes game outcomes are independent. In practice, opening choice and network determinism can introduce correlations between games. Paired openings mitigate but do not eliminate this.
- The opening book used for paired openings is an open item and will be addressed in a separate ADR.
- False positive and false negative rates of 5% and 1% respectively are standard but not validated for Aleph's specific training setup. They may require adjustment based on observed promotion behavior during training.