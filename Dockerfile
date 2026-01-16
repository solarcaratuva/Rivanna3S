FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc-arm-none-eabi \
    binutils-arm-none-eabi \
    libnewlib-arm-none-eabi \
    libstdc++-arm-none-eabi-newlib \
    cmake \
    ninja-build \
    python3 \
    ca-certificates \
  && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /root/Rivanna2

WORKDIR /root

CMD ["sleep", "infinity"]
