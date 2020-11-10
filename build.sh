#!/bin/bash

export ROOT_PATH=$(pwd)
export PATH=${ROOT_PATH}/dependencies/Tapir-Meta/install/bin:${PATH}

rm -rf build/
mkdir -p build
pushd build > /dev/null
echo cmake -GNinja -DLLVM_DIR=${ROOT_PATH}/dependencies/Tapir-Meta/install/lib/cmake/llvm -DTAPIR=On -DCMAKE_BUILD_TYPE=Release ../
cmake -GNinja -DLLVM_DIR=${ROOT_PATH}/dependencies/Tapir-Meta/install/lib/cmake/llvm -DTAPIR=On -DCMAKE_BUILD_TYPE=Release ../
ninja
source ./scripts/setup.sh
cd tests/c
make
popd > /dev/null
