Getting Started
=======
Official supported environment for building and runing TAPAS is ubuntu 16.04. You have to run these commands to install required packages from ubuntu package repositories :
```
# apt-get install build-essential cmake sbt verilator
````

Build
=======
To build TAPAS you should run following commands :
```
$ git clone git@github.com/sfu-arch/TAPAS.git
$ cd TAPAS
$ git clone --recursive https://github.com/sfu-arch/Tapir-Meta.git
$ cd Tapir-Meta/
$ ./build.sh
$ source ./setup-env.sh
$ cd ..; mkdir build; cd build;
$ cmake -DLLVM_DIR=<your repository>/Tapir-Meta/tapir/build/lib/cmake/llvm/ -DTAPIR=ON ..
$ make
 ```

Running tests
=======
TAPAS comes with bunch of available tests. You can run and get the `.scala` for them this way:
```
$ # in your code repository
$ cd test/c/
$ make all
```
This is going to generate chisel `.scala` file for c files in `test/c` directory.

How to run generator on your code (detailed way)?
=======
For generating `.scala` for your code you have to follow this steps :
1. Emit the llvm ir (`.ll`) for your code with supported Tapir compiler (`<your repository code location>/Tapir-Meta/tapir/build/bin/clang`).
    * You can out put llvm ir with this command `./Tapir-Meta/tapir/build/bin/clang -emit-llvm [Your source code]`
2. Run `opt` with `-mem2reg -loop-simplify -loop-simplifycfg -disable-loop-vectorization -dce` arguments on your llvm ir (`.ll`) code.
3. Run TAPAS generator on your `.ll` file like this :
    * `<your repository>/build/bin/dandelion -fn-name=[output file of last step] -config=../../scripts/config.json -o output.scala`

How to run generator on your code (simple way)?
=======
You can simply put your `.c` file in the `<your repository>/test/c` directory and simply run :
```
$ make all
```
After that you should have the `.scala` file beside your `.c` file with a same name.

Help
=======
```
./bin/xketch -help

-fn-name:           Target function
-o:                 Generate Xketch file for the target function
-l-ex:<false/true>  Extracting the loops of the function
-aa-Trace:          Printing memory traces
```

Extra Documentation
=======
For more information you can look at the wiki.
