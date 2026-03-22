# Architecture

This section documents how Aleph's submodules relate to each other and how the system behaves as a whole. Where the subsystems section covers the internals of each individual submodule, this section covers the relationships between them — how a position flows through the system during inference, how the training loop spans multiple submodules, how concurrency is structured, and the decisions that shaped the overall design. Architecture Decision Records are also collected here.

```{toctree}
:maxdepth: 1
:caption: Architecture

component-diagram
data-flow
training-loop
concurrency
adr/index
```