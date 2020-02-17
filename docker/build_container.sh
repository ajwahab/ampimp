#!/bin/bash

PROJECT_NAME='ampimp'
CONTAINER_NAME=$PROJECT_NAME
IMAGE_NAME="$(whoami)/${CONTAINER_NAME}"

if [ ! "$(docker ps -q -f name=${CONTAINER_NAME})" ]; then
    if [ "$(docker ps -aq -f status=exited -f name=${CONTAINER_NAME})" ]; then
      docker rm ${CONTAINER_NAME}
    fi
    if [[ -n "$( docker images -q ${IMAGE_NAME} )" ]]; then
      docker rmi $IMAGE_NAME
    fi
    docker build -t $IMAGE_NAME .

    docker run \
      --name ampimp \
      --rm
      -e HOST_IP=$(ifconfig en0 | awk '/ *inet /{print $2}') \
      -v ~/repos/$PROJECT_NAME:/home/repos/$PROJECT_NAME \
      -t -i \
      --privileged \
      --device /dev/tty.usbmodem145402 \
      $IMAGE_NAME
fi
