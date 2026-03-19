#!/bin/bash -eu

cd $SRC/aleph

# Convert space-separated CXXFLAGS/CFLAGS into Conan-compatible TOML list format
FLAGS_PY="import sys; flags=sys.argv[1].split(); print('[' + ', '.join(f\"'{f}'\" for f in flags if f) + ']')"

CONAN_CFLAGS=$(python3 -c "$FLAGS_PY" "$CFLAGS")
CONAN_CXXFLAGS=$(python3 -c "$FLAGS_PY" "$CXXFLAGS")
CONAN_LDFLAGS=$(python3 -c "$FLAGS_PY" "$LIB_FUZZING_ENGINE $LDFLAGS")

conan build . \
    --build=missing \
    -s "compiler.version=21" \
    -s "compiler.cppstd=20" \
    -c "tools.build:cflags=$CONAN_CFLAGS" \
    -c "tools.build:cxxflags=$CONAN_CXXFLAGS" \
    -c "tools.build:exelinkflags=$CONAN_LDFLAGS" \
    -c "tools.build:sharedlinkflags=$CONAN_LDFLAGS"