FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV PATH="/opt/arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi/bin:${PATH}"

RUN apt-get update && apt-get install -y --no-install-recommends \
    wget \
    tar \
    cmake \
    ninja-build \
    python3 \
    ca-certificates \
  && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /opt && \
    wget -O /tmp/arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi.tar.xz \
      https://developer.arm.com/-/media/Files/downloads/gnu/15.2.rel1/binrel/arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi.tar.xz && \
    tar -xJf /tmp/arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi.tar.xz -C /opt && \
    rm /tmp/arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi.tar.xz

RUN mkdir -p /root/Rivanna2

WORKDIR /root

CMD ["sleep", "infinity"]
