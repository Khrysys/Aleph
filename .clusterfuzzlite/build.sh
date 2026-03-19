#!/bin/bash -eu

# build project
# e.g.
# ./autogen.sh
# ./configure
# make -j$(nproc) all

cd $SRC/aleph

conan build . \
    --build=missing \
    -c tools.build:cxxflags=$CXXFLAGS
    -c tools.build:compiler_executables=$CXX
    -c tools.cmake.cmaketoolchain:extra_variables={'Aleph_BUILD_FUZZING', 'ON'}

# build fuzzers
# e.g.
# $CXX $CXXFLAGS -std=c++11 -Iinclude \
#     /path/to/name_of_fuzzer.cc -o $OUT/name_of_fuzzer \
#     $LIB_FUZZING_ENGINE /path/to/library.a
