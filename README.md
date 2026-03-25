# Aleph

[![OpenSSF Scorecard](https://api.scorecard.dev/projects/github.com/Khrysys/Aleph/badge)](https://scorecard.dev/viewer/?uri=github.com/Khrysys/Aleph) [![OpenSSF Best Practices](https://www.bestpractices.dev/projects/12194/badge)](https://www.bestpractices.dev/projects/12194) [![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

Aleph is a chess engine combining a cross-attention transformer evaluator with a
search-contempt MCTS algorithm. It is designed to be strong and efficient, built
on modern research in computer chess.

The name "Aleph" refers first to Aleph-Null, the smallest infinity from set
theory in mathematics, and is second a nod to AlphaZero and Leela Chess Zero,
whose empirical data informed the decisions for the neural network architecture.

## Status

Aleph is currently under extremely early development. The code within this repo is not a functional engine yet. More is written than is shown here, however, I am taking time to ensure a high standard before adding code from my local copy into `staging`, and then an even higher quality before adding it into the `main` branch. Often the `staging` branch is broken. However, `main` is always functional.

## Requirements

Aleph requires no special runtime dependencies beyond the standard C++20-compatible runtime. A GPU is optional, however CPU-only usage of the neural network is considerably slower and worse-performing than the GPU-accelerated performance. 

## Building 

See [./docs/src/developer/building.md](./docs/src/developer/building.md).

## Security

See [./SECURITY.md](./SECURITY.md).

## Contributing

See [./CONTRIBUTING.md](./CONTRIBUTING.md).

## Code of Conduct

See [./CODE_OF_CONDUCT.md](./CODE_OF_CONDUCT.md).

## Acknowledgements

The initial inspiration for this project came from reading through Lc0's update about transformers [here](https://lczero.org/blog/2024/02/transformer-progress/).

See [./docs/appendix/acknowledgements.md](./docs/appendix/acknowledgements.md).

## License

Aleph is licensed under the GPLv3 license. See [./LICENSE](./LICENSE).