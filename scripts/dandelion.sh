#!/usr/bin/env bash

# ----------------------------------
# Colors
# ----------------------------------
NOCOLOR='\033[0m'
RED='\033[0;31m'
GREEN='\033[0;32m'
ORANGE='\033[0;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
LIGHTGRAY='\033[0;37m'
DARKGRAY='\033[1;30m'
LIGHTRED='\033[1;31m'
LIGHTGREEN='\033[1;32m'
YELLOW='\033[1;33m'
LIGHTBLUE='\033[1;34m'
LIGHTPURPLE='\033[1;35m'
LIGHTCYAN='\033[1;36m'
WHITE='\033[1;37m'


# ----------------------------------
# GIT
# ----------------------------------
CHISEL_GIT="https://github.com/freechipsproject/chisel3.git"
FIRRTL_GIT="https://github.com/freechipsproject/firrtl.git"
CHISEL_TESTERS_GIT="https://github.com/freechipsproject/chisel-testers.git"
BERKELEY_HARD_FLOAT_GIT="https://github.com/ucb-bar/berkeley-hardfloat.git"
DSPTOOLS_GIT="https://github.com/ucb-bar/dsptools.git"
TAPIR_META="https://csil-git1.cs.surrey.sfu.ca/Dandelion/Tapir-Meta.git"

DEPEN="dependencies"

# ----------------------------------
# FUNCTIONS
# ----------------------------------

function mill_publish(){
    make mill.publishLocal
}

function sbt_publish(){
    sbt publishLocal
}

function build_verilator(){
    if ! hash verilator 2>/dev/null; then
        echo -e "${GREEN} Building verilator...${NOCOLOR}"
        mkdir -p ${DEPEN}
        pushd ${DEPEN} > /dev/null
        git clone "http://git.veripool.org/git/verilator"
        cd verilator
        git pull
        git checkout verilator_4_016
        unset VERILATOR_ROOT
        autoconf
        ./configure
        sudo make install

        popd > /dev/null
    fi

}

function git_clone(){
    mkdir -p ${DEPEN}
    pushd ${DEPEN} > /dev/null

    echo -e "${GREEN}Building: ${RED}$1....${NOCOLOR}"
    if [ -d $1 ]; then
        pushd $1 > /dev/null
    else
        git clone $2 $1
        pushd $1 > /dev/null
    fi

    sbt_publish
    popd > /dev/null
    popd > /dev/null

    echo "clone: " $2
}

function build_tapir(){
    mkdir -p ${DEPEN}
    pushd ${DEPEN} > /dev/null

    git clone --recursive ${TAPIR_META}
    pushd tapir_meta > /dev/null
    ./build.sh release
    popd > /dev/null

    popd > /dev/null
}

function build_dependencies(){
    echo -e "${GREEN}Installing dependencies, it needs sudo access to update packages...${NOCOLOR}"
    sudo apt update
    sudo apt install build-essential cmake libjsoncpp-dev  libncurses5-dev graphviz binutils-dev gcc-8-multilib g++-8-multilib apt install git make autoconf g++ flex bison -y
    echo -e "${GREEN}Dependencies installed sucessfully...${NOCOLOR}"
}


# ----------------------------------
# SCRIPTS
# ----------------------------------

#1) Clone and build all the dependencies
build_dependencies
build_verilator
build_tapir
echo -e "${GREEN}Buidling chisel dependencies...${NOCOLOR}"
git_clone "firrtl" ${FIRRTL_GIT}
git_clone "chisel3" ${CHISEL_GIT}
git_clone "chisel-testers" ${CHISEL_TESTERS_GIT}
git_clone "hardfloat" ${BERKELEY_HARD_FLOAT_GIT}
git_clone "dsptools" ${DSPTOOLS_GIT}
