# Contributing to Aleph

Thank you for your interest in contributing to Aleph. Please read this document
before opening issues or submitting pull requests. All contributors are expected
to follow the [Code of Conduct](./CODE_OF_CONDUCT.md).

## Branch Flow

Aleph uses a two-branch model:

- `staging` — integration branch. This branch may be broken at any time.
- `main` — stable branch. This branch is always functional.

All pull requests must target `staging`. **Pull requests targeting `main` directly
will not be accepted**, with the sole exception of PRs promoting
`staging` to `main`. This is a maintainer-only operation.

## Pull Request Title Conventions

Pull request titles must follow conventional commit style using one of the
following prefixes:

- `feat` — a new feature
- `fix` — a bug fix
- `chore` — maintenance tasks that do not affect functionality
- `docs` — documentation changes only
- `refactor` — code restructuring without behavior change
- `perf` — performance improvements
- `test` — adding or updating tests
- `ci` — changes to CI configuration or workflows
- `build` — changes to the build system or dependencies
- `release` — maintainer-only promotion of `staging` to `main`

Example: `feat: add PEXT hardware path to platform module`

## Issues and Pull Request Labels

Issues and pull requests are labeled by project area. Project areas include all
code modules as well as documentation and CI. New contributor-friendly issues are
labeled `good first issue`.

## Code Review

All pull requests require approval from Code Owners before merging. The Code
Owners file covers the entire repository. Approval from a Code Owner constitutes
review of both correctness and adherence to project standards.

## Code Style

Aleph follows a documented code style. See the [style guide](./docs/src/developer/style.md)
for full details. <!-- Note: style guide is currently a placeholder -->

## Test Policy

All major new functionality added to Aleph must be accompanied by tests in the
automated test suite. This applies to new features, bug fixes that expose missing
coverage, and refactors that change behavior. Pull requests that add functionality
without corresponding tests will not be accepted.

## Building

See [docs/source/developer/building.md](./docs/src/developer/building.md) for
full build instructions including Conan and CMake setup.

## Submitting a Pull Request

1. Fork the repository and create a branch from `staging`, not `main`
2. Make your changes, ensuring all existing tests pass
3. Add or update tests as appropriate for your change
4. Follow the PR title conventions above
5. Open a pull request targeting `staging`
6. Address any review feedback from Code Owners
7. Once approved, your PR will be merged by a maintainer