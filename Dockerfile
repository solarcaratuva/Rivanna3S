FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV PATH="/opt/gcc-arm-none-eabi/bin:${PATH}"

RUN apt-get update && apt-get install -y --no-install-recommends \
    wget \
    tar \
    cmake \
    ninja-build \
    python3 \
    ca-certificates \
  && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /opt && \
    wget -O /tmp/gcc-arm-none-eabi.tar.xz https://developer.arm.com/-/media/Files/downloads/gnu-rm/12.3-2023.05/gcc-arm-none-eabi-12.3-2023.05-x86_64-linux.tar.bz2 && \
    tar -xjf /tmp/gcc-arm-none-eabi.tar.xz -C /opt && \
    rm /tmp/gcc-arm-none-eabi.tar.xz

RUN mkdir -p /root/Rivanna2

WORKDIR /root

CMD ["sleep", "infinity"]
