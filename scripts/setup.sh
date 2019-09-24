#!/usr/bin/env bash


DANDELION_PATH=@CMAKE_BINARY_DIR@/bin
DANDELION_SCRIPTS=@CMAKE_BINARY_DIR@/scripts

source ${DANDELION_SCRIPTS}/colors.sh

echo -e "${GREEN}Dandelion is intalled in: ${RED}${DANDELION_PATH}${NOCOLOR}"
[[ ":$PATH:" != *":${DANDELION_PATH}:"* ]] && PATH="${DANDELION_PATH}:${PATH}"
echo -e "${PURPLE}Your PATH is set!${NOCOLOR}"

