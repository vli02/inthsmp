#!/bin/bash

did=$(docker ps --filter "ancestor=toolchain:alpine" -q)

if [ -z $did ]; then
    echo "Start toolchain container."
    bin_dir=$(dirname "$0")
    cd "$bin_dir"
    top_dir="$(git rev-parse --show-toplevel)"
    docker run --cap-add=SYS_PTRACE --security-opt seccomp=unconfined \
               --init -i$([ -t 0 ] && echo t) -u $(id -u):$(id -g) \
               -v "$top_dir":/inthsmp -w /inthsmp toolchain:alpine
    echo "Terminate toolchain container."
else
    echo "Exec into toolchain container."
    docker exec -i$([ -t 0 ] && echo t) -u $(id -u):$(id -g) -w /inthsmp $did /bin/sh -l
    echo "Exit toolchain container."
fi
