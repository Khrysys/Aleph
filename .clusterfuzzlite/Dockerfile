FROM gcr.io/oss-fuzz-base/base-builder:v1
RUN apt-get update && apt-get install -y ninja-build cmake python3-pip && pip3 install conan && conan profile detect
COPY . $SRC/aleph
WORKDIR aleph
COPY .clusterfuzzlite/build.sh $SRC/
