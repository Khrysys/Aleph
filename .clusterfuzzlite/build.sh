#!/bin/bash -eu

cd $SRC/aleph

conan build . \
    --build=missing \
    -s compiler.version=21 \
    -s compiler.cppstd=20 \
    -c tools.build:cxxflags=\"$CXXFLAGS\" \
    -c tools.build:compiler_executables=\"$CXX\" \
    -c tools.cmake.cmaketoolchain:extra_variables={'Aleph_BUILD_FUZZING', 'ON'}