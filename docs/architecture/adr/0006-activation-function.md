# 0006 - SiLU Activation

## Context

Aleph's pre and post projections require a nonlinear activation function. Three candidates were considered: ReLU, GELU, and SiLU.

**ReLU** is the simplest option and historically the default in deep learning. Its primary practical problem is dead neurons: any unit whose pre-activation is consistently negative receives zero gradient and stops learning entirely. In a network without batch normalization or careful initialization, dead ReLU units can silently degrade capacity. ReLU also has a hard zero at the origin, producing a non-smooth gradient that can introduce instability in deeper networks.

**GELU** is the current dominant activation in transformer literature, used in GPT, BERT, and most of their descendants. Like SiLU, it has no trainable parameter. It is smooth, non-monotonic, and avoids dead neurons. The practical difference between GELU and SiLU is small — both produce similar results on most tasks, and which converges faster depends on initialization, input distribution, and the shape of the loss landscape rather than any inherent superiority of one over the other. There is no strong empirical basis for preferring one unconditionally.

**SiLU** (Sigmoid Linear Unit) is equivalent to Swish with a fixed beta of 1 — that is, `x * sigmoid(x)`. It is smooth, non-monotonic, and has a small negative region near zero that allows gradients to flow even for negative pre-activations. Its compute cost is marginally lower than GELU's.

## Decision

Aleph uses SiLU on all pre and post projections. ReLU was rejected due to the dead neuron problem. GELU and SiLU are both reasonable choices with no strong empirical basis for preferring one over the other unconditionally; SiLU was chosen for its marginally lower compute cost and the simplicity of its formulation.

### Interaction with the projection pattern

SiLU is never applied directly before attention or a residual connection. Its output range is approximately [slightly negative, +∞), which is not suitable as a direct input to scaled dot-product attention or as a residual update. In Aleph's pattern, SiLU is always followed by a linear output projection, which maps back to an unconstrained range (−∞, +∞) before the result is used in attention or added to the residual. This mirrors the behavior of a standard ReLU FFN in a transformer — nonlinearity followed by a linear layer that restores the full output range — but without the FFN's dedicated sub-layer.

## Consequences

**Positive**

- No dead neuron problem. All units receive gradient regardless of pre-activation sign.
- Smooth gradient everywhere, supporting stable training in deeper networks.
- Negligible compute overhead relative to GELU.
- Fixed beta=1 keeps the activation simple and removes a tuning surface with no practical upside.

**Negative**

- SiLU's non-monotonicity is occasionally cited as a potential source of optimization difficulty, though this has not been observed to be a practical issue in transformer training.
- The small negative output region means pre-projection outputs are not strictly non-negative, which would matter for certain downstream operations. In Aleph's case this is not a concern since SiLU output is always consumed by a subsequent linear projection.