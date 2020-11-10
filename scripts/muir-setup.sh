#!/usr/bin/env bash

set -e

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
TAPIR_META="https://github.com/sfu-arch/Tapir-Meta.git"

DEPEN="dependencies"

# ----------------------------------
# FUNCTIONS
# ----------------------------------

progress-bar() {
  local duration=${1}


    already_done() { for ((done=0; done<$elapsed; done++)); do printf "▇"; done }
    remaining() { for ((remain=$elapsed; remain<$duration; remain++)); do printf " "; done }
    percentage() { printf "| %s%%" $(( (($elapsed)*100)/($duration)*100/100 )); }
    clean_line() { printf "\r"; }

  for (( elapsed=1; elapsed<=$duration; elapsed++ )); do
      already_done; remaining; percentage
      sleep 1
      clean_line
  done
  clean_line
}

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

    git clone --branch release_60 --recursive ${TAPIR_META}
    pushd Tapir-Meta > /dev/null
    ./build.sh release
    popd > /dev/null

    popd > /dev/null
}

function build_dependencies(){
    echo -e "${GREEN}Installing dependencies, it needs sudo access to update packages...${NOCOLOR}"
    sudo apt update
    sudo apt install -y build-essential cmake libjsoncpp-dev libncurses5-dev graphviz binutils-dev gcc-8-multilib g++-8-multilib libfl2 libfl-dev git make autoconf g++ flex bison python gpg default-jdk python3 python3-dev python3-pip gcc libtinfo-dev zlib1g-dev ninja-build libsnappy-dev linux-libc-dev:i386
    sudo rm /usr/bin/g++
    sudo ln -s /usr/bin/g++-8 /usr/bin/g++
    echo "deb https://dl.bintray.com/sbt/debian /" | sudo tee -a /etc/apt/sources.list.d/sbt.list
    sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv 642AC823
    sudo apt update
    sudo apt install sbt -y

    echo -e "${GREEN}Dependencies installed sucessfully...${NOCOLOR}"
}

# ----------------------------------
# SCRIPTS
# ----------------------------------

echo -e "${GREEN}Setting up muIR dependencies.......${NOCOLOR}"

#1) Clone and build all the dependencies
while true; do
    read -p "Do you wish to install muIR dependencies? [y/N]" yn
    case ${yn:-N} in
        [Yy]* ) build_dependencies; break;;
        [Nn]* ) break;;
        * ) echo "Please answer yes or no.";;
    esac
done

# Installing verilator
while true; do
    read -p "Do you wish to install Verilator? [y/N]" yn
    case ${yn:-N} in
        [Yy]* ) build_verilator; break;;
        [Nn]* ) break;;
        * ) echo "Please answer yes or no.";;
    esac
done

while true; do
    read -p "Do you wish to install LLVM/Tapir? [y/N]" yn
    case ${yn:-N} in
        [Yy]* ) build_tapir; break;;
        [Nn]* ) break;;
        * ) echo "Please answer yes or no.";;
    esac
done

