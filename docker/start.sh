#!/bin/bash

bin_dir=$(dirname "$0")

cd "$bin_dir"

top_dir="$(git rev-parse --show-toplevel)"

docker run --init -i$([ -t 0 ] && echo t) -u $(id -u):$(id -g) -v "$top_dir":/inthsmp -w /inthsmp toolchain:alpine
