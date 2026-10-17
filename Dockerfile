FROM debian:trixie-slim
RUN apt-get update && apt-get install -y --no-install-recommends \
    clang \
    clang-format \
    lld \
    llvm \
    make \
    binutils \
    file \
    genisoimage \
    python3-pytest \
    python3-pycdlib \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
CMD ["make"]
