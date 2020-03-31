# muIR-Generator

[![CircleCI](https://circleci.com/gh/sfu-arch/muir.svg?style=svg)](https://circleci.com/gh/sfu-arch/muir)
[![Gitter](https://badges.gitter.im/sfu-arch/community.svg)](https://gitter.im/sfu-arch/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)
[![](https://tokei.rs/b1/github/sfu-arch/muir)](https://github.com/sfu-arch/muir)


muIR-Generator is a tool to generator hardware accelerator from software programs. muIR-Generator uses muIR as an intermediate representation (IR) to design hardware accelerators. Currently, muIR-Generator supports C/C++ and Cilk programs.

## Getting Started

Official supported environment for building and running muIR-Generator is ubuntu 18.04.

**Step One: Building LLVM/TAPIR-6:**  muIR-Generator uses LLVM/TAPIR compiler (LLVM compiler with parallel instruction extension to support Cilk programs). The following link contains a forked version of of [LLVM/TAPIR-6](https://github.com/sfu-arch/Tapir-Meta) with a minor changes which muIR is compatible with.
To ease of building TAPIR, we have scripted the build process of TAPIR:

``` bash
git clone https://github.com/sfu-arch/muir.git
cd muir
./scripts/dandelion.sh
```

**Step Two: Building muIR-Generator:** Now we have installed and built all the muIR-Generator dependencies we can build the project:

```bash
mkdir build
cd build
cmake -DLLVM_DIR=<your_repository_path>/dependencies/Tapir-Meta/tapir/build/lib/cmake/llvm/ -DTAPIR=ON ..
make -jN
```

**Step Three: Setting your PATH:** To set your ``PATH`` variable you can use the following script which is under **build** directory:

``` bash
source ./scripts/setup-env.sh
```

After sourcing the script you should see the following message;

```
Dandelion is installed in:<your_repository_path>/build/bin
Your PATH is set!
```

## Running test cases

Under *muir/tests* directory there are muir examples and the application examples which muIR framework is tested with.
To keep the build process integrated and simple, muIR cmake file copy all these test cases under *build/tests* and make sure all the environment variables under this directory is properly set:

```bash
cd <your_repository>/build/tests/c/
make
```

**NOTE:** Currently, for simplicity of the Makefile in each example the target function's name needs to be the same name as the source file's name, hence, the Makefile can pick the right values for the compilation process.

## Testing the generated hardware accelerators

After running the ``make`` command for each test case there should be one Scala file. This Scala file is the target function accelerator description id muIR.
muIR-Generator uses [muIR-Lib](https://github.com/sfu-arch/muir-lib) chisel library to describe hardware accelerator.
To be able to run and test the hardware accelerator, currently, we support the following back-ends:

* [muIR-Sim](https://github.com/sfu-arch/muir-sim)
* [muIR-F1](https://github.com/amsharifian/dandelion-aws)
* [muIR-SoC](https://github.com/sfu-arch/muir-fpga)
* [muIR-V](https://github.com/amsharifian/rocket-rocc-examples)


Each project, separately has tutorial on how to connect the generated hardware accelerator design and run the full-system application.

## Need more details on muIR project and implementation?

In the following repo, [Dandelion-Tutorial](https://github.com/amsharifian/dandelion-tutorial), we are documenting all the Dandelion project pieces, muIR is one of the subprojects of Dandelion.
To have more information on the design details and other project you can follow the documentation.

## How to run generator on your code (detailed way)?

**This section is not complete yet**

<!-- For generating `.scala` for your code the following steps need to be taken :

1. Emit the llvm ir (`.ll`) for your code with supported Tapir/dandelion-generator compiler(`<your repository code location>/Tapir-Meta/tapir/build/bin/clang`).
    * You can out put llvm ir with this command `./Tapir-Meta/tapir/build/bin/clang -emit-llvm [Your source code]`
2. Run `opt` with `-mem2reg -loop-simplify -loop-simplifycfg -disable-loop-vectorization -dce` arguments on your llvm ir (`.ll`) code.
3. Run TAPAS generator on your `.ll` file like this :
    * `<your repository>/build/bin/dandelion -fn-name=[output file of last step] -config=../../scripts/config.json -o output.scala`
 -->

Help
=======
```
./bin/dandelion -help

dandelion options:

  -aa-trace                - Alias analysis trace
  -config=<config_file>    - Target function name
  -fn-name=<Function name> - Target function name
  -l-ex                    - Extracting loops
  -o=<filename>            - tapas output file
  -test-file               - Printing Test file

Generic Options:

  -help                    - Display available options (-help-hidden for more)
  -help-list               - Display list of available options (-help-list-hidden for more)
  -version                 - Display the version of this program
```

Extra Documentation
===================
For more information you can look at the wiki.
https://github.com/sfu-arch/dandelion-lib/wiki


Author:
========
* Amirali Sharifian (amiralis@sfu.ca)
