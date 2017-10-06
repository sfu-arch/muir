These programs are demonstrations of how LLVM can be used for (very simple)
static and dynamic analyses. However, they use many of the initial building
blocks necessary for solving more complex problems.

The provided analyses count the number of direct invocations of a particular
function are present within a program or within and execution depending on
whether static or dynamic analysis is used.

These instructions assume that your current directory starts out as the "demo"
directory within the package.


Building with CMake
==============================================
1. Clone the repository.

        git clone git@csil-git1.cs.surrey.sfu.ca:amoeba/xketch-generator.git

2. Create a new directory for building.

        mkdir build

3. Change into the new directory.

        cd build

4. Run CMake with the path to the LLVM source.

        cmake -DLLVM_DIR=</path/to/LLVM/build>/lib/cmake/llvm/ ../

5. Run make inside the build directory:

        make

This produces a callcounter tool called bin/callcounter and supporting
libraries in lib/.

Note, building with a tool like ninja can be done by adding `-G Ninja` to
the cmake invocation and running ninja instead of make.

Running
==============================================

First suppose that you have a program compiled to bitcode:

    clang -emit-llvm -m32 -S loop.c -o - | opt -mem2reg -dot-cfg -o loop.bc

Runing the generator:

    ./bin/xketch ../test/loop.bc -fn-name=foo -o test-for.scala && llvm-dis final.bc

    -fn-name:           Target function
    -l-ex:<false/true>  Extracting the loops of the function
    -aa-Trace:          Printing memory traces


