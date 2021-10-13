#!/bin/bash

#Ensure nothing happens outside the directory this script is ran from
cd "$(dirname "$0")"
SCRIPT_DIRECTORY=$(pwd)

ARCHITECTURE=""
case $(uname -m) in
    i386)   ARCHITECTURE="386" ;;
    i686)   ARCHITECTURE="386" ;;
    x86_64) ARCHITECTURE="amd64" ;;
    arm)    dpkg --print-ARCHITECTURE | grep -q "arm64" && ARCHITECTURE="arm64" || ARCHITECTURE="arm" ;;
esac

echo "[INFO] Processor Architecture Detected as $ARCHITECTURE"

DOCKER_BASE_IMAGE="ubuntu:18.04"
DOCKER_FINAL_IMAGE_TAG="dorowu/ubuntu-desktop-lxde-vnc:bionic"

# Comment or Uncomment as needed to build the appropriate final image
# DOCKER_BASE_IMAGE="ubuntu:20.04"
# DOCKER_FINAL_IMAGE_TAG="dorowu/ubuntu-desktop-lxde-vnc:latest"

docker build -t "$DOCKER_FINAL_IMAGE_TAG" -f "Dockerfile.$ARCHITECTURE" \
                                          --build-arg DOCKER_BASE_IMAGE="$DOCKER_BASE_IMAGE" \
                                          $SCRIPT_DIRECTORY