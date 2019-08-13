# Dandelion Generator


[![pipeline status](https://csil-git1.cs.surrey.sfu.ca/Dandelion/dandelion-generator/badges/master/pipeline.svg)](https://csil-git1.cs.surrey.sfu.ca/Dandelion/dandelion-generator/commits/master)

Getting Started
=======
Official supported environment for building and running dandelion-generator is ubuntu 16.04. You have to run these commands to install required packages from ubuntu package repositories :
```
echo "deb https://dl.bintray.com/sbt/debian /" | sudo tee -a /etc/apt/sources.list.d/sbt.list
sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv 2EE0EA64E40A89B84B2DF73499E82A75642AC823
sudo apt-get update

apt-get install build-essential cmake sbt libjsoncpp-dev
````

Build
=======
To build dandelion-generator you should run following commands :
```
$ git clone git@csil-git1.cs.surrey.sfu.ca:Dandelion/dandelion-generator.git
$ cd dandelion-generator
$ git clone --recursive git@csil-git1.cs.surrey.sfu.ca:Dandelion/Tapir-Meta.git
$ cd Tapir-Meta/
$ ./build.sh
$ source ./setup-env.sh
$ cd ..; mkdir build; cd build;
$ cmake -DLLVM_DIR=<your repository>/Tapir-Meta/tapir/build/lib/cmake/llvm/ -DTAPIR=ON ..
$ make
 ```

Running tests
=======
Inside test directory there are set of test example which show the generality of dandelion-generator.
To get the generated accelerator files for test cases you can run:

```
$ # in your code repository
$ cd test/c/
$ make all
```

For each test case there is going to be one sacala file which has the detailed implementation of dandelion-generator.

How to run generator on your code (detailed way)?
=======
For generating `.scala` for your code the following steps need to be taken :

1. Emit the llvm ir (`.ll`) for your code with supported Tapir/dandelion-generator compiler(`<your repository code location>/Tapir-Meta/tapir/build/bin/clang`).
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
=======
For more information you can look at the wiki.
https://github.com/sfu-arch/dandelion-lib/wiki
