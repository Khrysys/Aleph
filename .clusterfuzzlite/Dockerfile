FROM gcr.io/oss-fuzz-base/base-builder@sha256:cc6982a6ce8b02c80a2acabbcfe766e5e7200988ff0424b0d4963232c3b41901

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    apt-get update && apt-get install -y --no-install-recommends \
    cmake libstdc++-10-dev python3-pip
    
COPY . $SRC/aleph
WORKDIR $SRC/aleph
COPY .clusterfuzzlite/build.sh $SRC/