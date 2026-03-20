FROM gcr.io/oss-fuzz-base/base-builder:v1@sha256:cc6982a6ce8b02c80a2acabbcfe766e5e7200988ff0424b0d4963232c3b4190

RUN apt-get update && apt-get install -y cmake python3-pip libstdc++-10-dev

COPY . $SRC/aleph

RUN pip3 install -r "$SRC/aleph./.github/requirements/conan.txt" --require-hashes && conan profile detect

WORKDIR $SRC/aleph
COPY .clusterfuzzlite/build.sh $SRC/
