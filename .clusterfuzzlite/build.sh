#!/bin/bash -eu

cd $SRC/aleph

conan build . \
    --build=missing \
    --profile=.github/profiles/fuzzing-host

find build -name "fuzz_*" -type f -executable | xargs -I{} cp {} $OUT/