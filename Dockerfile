FROM debian:trixie-slim
RUN apt-get update && apt-get install -y --no-install-recommends \
    clang \
    lld \
    llvm \
    make \
    binutils \
    file \
    genisoimage \
    python3 \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
CMD ["make"]
