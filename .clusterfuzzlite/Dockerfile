FROM gcr.io/oss-fuzz-base/base-builder:v1

RUN apt-get update && apt-get install -y cmake python3-pip libstdc++-10-dev

COPY . $SRC/aleph
WORKDIR $SRC/aleph
COPY .clusterfuzzlite/build.sh $SRC/
