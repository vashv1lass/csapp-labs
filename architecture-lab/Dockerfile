# Just a copy of a Dockerfile from Attack Lab.

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    lldb \
    gcc-multilib \
    make \
    libc6-dev-i386 \
    wget \
    curl \
    hexedit \
    nano \
    vim \
    flex \
    bison \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /lab