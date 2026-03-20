FROM gcr.io/oss-fuzz-base/base-builder:v1

RUN apt-get update && apt-get install -y cmake python3-pip libstdc++-10-dev

COPY . $SRC/aleph

RUN pip3 install -r "$SRC/aleph./.github/requirements/conan.txt" --require-hashes && conan profile detect

WORKDIR $SRC/aleph
COPY .clusterfuzzlite/build.sh $SRC/
