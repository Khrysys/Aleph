# Aleph (ℵ₀)

Aleph is a C++20 chess engine combining search-contempt Monte Carlo Tree Search (arXiv:2504.07757) with a cross-attention transformer evaluator. The evaluator uses a board self-attention encoder and a move encoder, bridged by cross-attention, to produce a policy over legal moves, a win/draw/loss value estimate, and a moves-left head. The entire stack — architecture, training loop, and self-play — is implemented in C++ with LibTorch; there is no Python training path.

```{toctree}
:maxdepth: 2
:caption: Contents

basic/index
advanced/index
developer/index
api-reference/index
appendix/index
```

## Documentation Structure

This documentation is broken up into five main categories:

- A section for users who may not have a computer science background. This segment will have all the documentation you need to understand the interfaces of Aleph and potentially a bit of how it works under the hood. The only assumption is an understanding of basic chess rules, and even that is mostly superficial in order to better understand certain components.
- A section for non-maintainer programmers. This segment will likely require for you to have a programming background at a minimum, but it does attempt to keep the concepts simple enough that someone fresh out of a high school robotics or computer science course could have familiar concepts to understand how Aleph truly works from a programming standpoint. As an example, some of the bit tricks utilized in the chess engine will be explained here, but the vast majority will be left to the following section and simply hand-waved here, for the sake of clarity. 
- A section for maintainers and potential contributors. This segment will go into each module in depth, but not into every individual function's workings. I highly recommend for you to have read the earlier sections before getting here, otherwise some things may not be explained. Expect some high level math to appear for certain bit tricks and ideas. 
- An API reference. This mirrors the comments in the code itself, as it is extracted from the code via Breathe to be added to the Sphinx documentation here. I have attempted to make all of the internal workings of the non-trivial functions laid out explicitly, but I only have so much time and so some may be questionably effective. 
- An appendix, specifically with a glossary to assist with the earlier sections, and also for simply miscellaneous files that don't really belong to any other single section. 

My hope with this is that the documentation laid out in these sections will allow for anyone who could happen across this engine to be able to use it and understand it at least at the surface level. 