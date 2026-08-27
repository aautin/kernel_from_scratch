FROM debian:bookworm-slim

RUN apt-get update && \
    apt-get install -y grub2-common xorriso grub-pc-bin
