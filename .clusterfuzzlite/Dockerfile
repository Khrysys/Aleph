FROM gcr.io/oss-fuzz/base-builder-clang

RUN apt-get update && apt-get install -y python3-pip
RUN pip3 install conan --break-system-packages

COPY . $SRC/aleph
WORKDIR $SRC/aleph