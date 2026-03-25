# Requirements

- C++20-compatible compiler
- Python 3 (for Conan)
- Conan package manager

It is also technically possible to build Aleph without Conan, however it is very much not recommended; Conan handles many complex functions for dependencies automatically, which Aleph depends on. Issues not utilizing Conan will not be fixed unless the problem is extremely clear.

It is recommended to have a version of CMake installed, however this is not required; in most circumstances Conan handles CMake for building automatically.

Conan utilizes profiles in order to tell CMake what to do. It will not work without one. A basic profile can be created by running:

```bash
conan profile detect
```

This profile most likely will have the `compiler.cppstd` set to a value below `20` by default. It is recommended to copy this profile locally to `./profile.toml`, so that this value can be set to `20,` so that the engine will be built in C++20. 

# Building 

```bash
# Clone
git clone https://github.com/Khrysys/Aleph.git
cd Aleph
# Building the engine
conan build . --build=missing -pr profile.toml
```

This command will automatically download, build, and install all dependencies, then build Aleph, and finally run the tests for Aleph to ensure everything is working properly. To prevent the tests from running automatically:

```bash
conan build . --build=missing -pr profile.toml -c tools.build:skip_test=True
```

This will still build the tests, but it will not run them automatically. To not build any tests:


```bash
conan build . --build=missing -pr profile.toml -c tools.build:skip_test=True -c tools.graph:skip_test=True
```

This will not build any test dependencies, including GoogleTest. 

# Documentation

The documentation is stored in `docs/`. This also stores documentation such as `security.md`, `code_of_conduct.md`, `contributing.md`, and more. The documentation uses Sphinx as the main build system, and Doxygen for the API docs with Breathe as an intermediary. The documentation can be built with:

```bash
cd docs
# Install dependencies
pip install -r requirements.txt

# Build docs 
make html
# Windows may want the following instead:
# ./make html
```