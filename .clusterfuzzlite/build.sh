#!/bin/bash -eu

cd $SRC/aleph

# Convert space-separated CXXFLAGS/CFLAGS into Conan-compatible TOML list format
FLAGS_PY="import sys; flags=sys.argv[1].split(); print('[' + ', '.join(f\"'{f}'\" for f in flags if f) + ']')"

CONAN_CFLAGS=$(python3 -c "$FLAGS_PY" "$CFLAGS")
CONAN_CXXFLAGS=$(python3 -c "$FLAGS_PY" "$CXXFLAGS")

conan build . \
    --build=missing \
    --build="b2/*" \
    -s:a "compiler.version=21" \
    -s "compiler.cppstd=20" \
    -c "tools.build:cflags=$CONAN_CFLAGS" \
    -c "tools.build:cxxflags=$CONAN_CXXFLAGS" \
    -o "boost:without_cobalt=True" \
    -o "boost:without_cobalt_io=True" \
    -o "boost:without_locale=True" \
    -o "boost:without_stacktrace_addr2line=True" \
    -o "boost:without_stacktrace_backtrace=True"