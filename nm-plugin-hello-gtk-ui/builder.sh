#!/bin/bash

mkdir -p ~/tmp-build

TMPDIR=~/tmp-build podman build --no-cache -f Containerfile -t gnome-dev .

TMPDIR=~/tmp-build podman run --rm -v "$PWD:/src:Z" gnome-dev \
  sh -c "cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ && cmake --build build"
#  sh -c "rm -rf build && cmake -B build && cmake --build build"
#
