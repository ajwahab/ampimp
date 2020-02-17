# Filename: Dockerfile

#FROM ubuntu:latest
#RUN apt-get update && apt-get install -y \
#  vim \
#  ssh \
#  git \
#  python3 \
#  python3-pip \
#  gdb \
#  cmake \
#  binutils \
#  gcc-arm-linux-gnueabi \
#  gcc-arm-none-eabi \
#  make \
#  libtool \
#  autoconf \
#  automake \
#  pkg-config \
#  texinfo \
#  libusb-dev \
#  libusb-1.0 \
#  libftdi-dev \
#  libhidapi-dev \
#&& rm -rf /var/lib/apt/lists/*

FROM alpine:edge
RUN apk add --no-cache \
  vim \
  openssh-client \
  git
RUN apk add --no-cache \
  gdb \
  cmake \
  build-base \
  screen
RUN apk add --no-cache -X http://dl-cdn.alpinelinux.org/alpine/edge/testing \
  binutils-cross-embedded
RUN apk add --no-cache \
  musl \
  make \
  libtool \
  autoconf \
  automake \
  pkgconf \
  texinfo \
  libusb-dev \
  libftdi1-dev \
  hidapi-dev

RUN apk add --no-cache --update python3 python3-dev\
&& pip3 install --upgrade pip setuptools
RUN pip3 install --user \
  pyserial
RUN export PATH="/root/.local/bin:$PATH"

WORKDIR /home/repos

RUN git clone --recurse-submodules https://github.com/ninfinity/openocd.git \
  && cd openocd \
  && ./bootstrap \
  && ./configure \
  && make \
  && make install \
  && cd .. \
&& rm -rf ./openocd
