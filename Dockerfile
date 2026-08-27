#environnement for building the OS

FROM debian:13

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends\
    nasm \
    build-essential \
    glibc-multilib \
    grub2-pc-bin \
    grub-common \
    xorriso \
    mtools \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /kfs-build