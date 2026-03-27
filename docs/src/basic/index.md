# Aleph for Players

```{toctree}
:maxdepth: 1
:caption: Contents

```

## A Brief History of Chess Engines

This section is wholy unnecessary for the rest of the documentation, but I thought it interesting enough to keep around, so here is what I believe is a relatively accurate history of chess engines. I'm obviously simplifying a lot and removing a considerable amount of other, extremely notable and important engines, but this is truly an unstoppable rabbit hole. I do however believe that especially for chess, you can learn a lot by looking at the past. I believe that it truly helps show my thinking with Aleph's architecture. 

### The Earliest Chess "Engines"

The earliest chess engines were not really chess engines, but more like a mechanical solver that could work for a specific set of states. For example <name here> was a chess "engine" that could deliver a checkmate with a rook and a king against a lone king every time. While more complicated than mating with a queen, which is often one of the first things a new player learns, it still leaves a lot to be desired in the way of generalization. Even before that was the Mechanical Turk, a claimed chess engine, but the logic was equivalent to "where do you hide the batteries" in supposed perpetual motion devices in the modern day. The Mechanical Turk just hid a grandmaster beneath the board in a box who played, and had some interesting mechanical linkages and a clever use of magnets. Again, far from being a true chess engine.

### The First Generalized Chess Engine

### Deep Blue

One of the more famous chess engines was Deep Blue, made by Googe DeepMind, their third foray into chess, who defeated World Champion Kasparov, widely considered to be one of the greatest players of all time, in a Classical World Championship-style match. Notably, this was not the first time a chess engine had beaten a grandmaster, nor even was it the first time it had beaten a World Champion, but simply the first time that it had beaten a human in a format that holds quite so much prestige in the world of chess. This was the first time that engines were truly better than humans at chess.

### CPU Engines

Up until 2017 or so, every chess engine was a CPU engine. This means that it parallelized its work across many CPU threads and kept its evaluations of a position hand written by humans, which were meant to be extremely cheap so the engine could process many hundreds of millions of positions per second per thread. Additionally, it is usually good a good coding principle to synchronize resource access across these threads (think of trying to read a paragraph but every second someone swaps the page for a completely different one), Stockfish, still to this day one of the strongest CPU engines, foregoes all synchronization in favor of performing an additional validation step on each thread. This allows for it to scale extremely well for a CPU engine.

### AlphaZero

### Modern GPU Engines

Soon after AlphaZero, Leela Chess Zero started as an open source variant. Over the years, they made numerous improvements to the design and it often competes directly with Stockfish, albeit it does still lose on average to it. They improved the time management by adding a `moves-left` output to the model directly, so that the network could directly control the time usage in a given position. An easy way to think about it is that, assuming the exact same board, a lower value will mean more time usage. 

### Aleph

While Leela Chess Zero may use a transformer model in their most recent versions, it is notably still something called "self-attention". This means that the transformer looks at the board to update the board. This is the same principle

While I have put a lot of effort into making Aleph as fast as possible, this project was written completely from scratch, with only the barest of references to other engines utilized in the `chess` and `caching` modules. This is much to the contrary of something like Stockfish, where it was based on Glarung 2.1, an earlier engine, and it also is a much older and well maintained engine, having hundreds of contributors. Even Leela Chess Zero, while it was one of the first engines to utilize a neural network, and therefore had no prior code aside from papers written by DeepMind on AlphaZero, it still was based heavily on AlphaZero, and its models used the structure laid out by AlphaZero up until 2022 when they began utilizing transformers. 