FROM gcr.io/oss-fuzz-base/base-builder:v1

RUN apt update && apt install -y cmake python3-pip && apt satisfy "ninja-build >= 1.11" && pip3 install conan && conan profile detect

COPY . $SRC/aleph
WORKDIR aleph
COPY .clusterfuzzlite/build.sh $SRC/
