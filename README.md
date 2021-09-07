About Patmos
============

Patmos is a time-predictable VLIW processor.
Patmos is the processor for the T-CREST project.
See also: http://www.t-crest.org/ and http://patmos.compute.dtu.dk/

The Patmos [Reference Handbook](http://patmos.compute.dtu.dk/patmos_handbook.pdf)
contains build instructions in Section 5.

For questions and discussions join the Patmos mailing list at:
https://groups.yahoo.com/group/patmos-processor/

Getting Started
===============

Several packages need to be installed.
The following apt-get lists the packages that need to be
installed on a Ubuntu Linux:

```
sudo apt install git openjdk-8-jdk gitk cmake make g++ texinfo flex bison \
  subversion libelf-dev graphviz libboost-dev libboost-program-options-dev ruby-full \
  liblpsolve55-dev python zlib1g-dev gtkwave gtkterm scala autoconf libfl2 expect
```

Make sure to use Java 8 and remove any later Java version with ```sudo apt autoremove```.


Install sbt according to the instructions from [sbt download](https://www.scala-sbt.org/download.html)

We assume that the T-CREST project will live in $HOME/t-crest.
Before building the compiler, add the path
to the compiler executables into your .bashrc or .profile:

    export PATH=$PATH:$HOME/t-crest/local/bin

Use an absolute path as LLVM cannot handle a path relative to the
home directory (~). Logout and login again to make your new PATH setting active.

In order to build the C++ emulator of Patmos, Verilator version 4.028 or higher must be installed.  
With Ubuntu 20.04 this can be installed through apt-get:

    sudo apt-get install verilator

Older versions of Ubuntu must install Verilator via their git repository:

    git clone https://github.com/verilator/verilator
    unset VERILATOR_ROOT
    cd verilator
    git checkout v4.028 #Tested version - newer should be fine
    autoconf
    ./configure
    make
    sudo make install

You can remove the verilator repository after installation



Patmos and the compiler can be checked out from GitHub and are built as follows:

    mkdir ~/t-crest
    cd ~/t-crest
    git clone https://github.com/t-crest/patmos-misc.git misc
    ./misc/build.sh

For developers with push permission the ssh based clone string is:

    git clone git@github.com:t-crest/patmos-misc.git misc

build.sh will checkout several other repositories (the compiler, library,
and the Patmos source) and
build the compiler and the Patmos simulator.
Therefore, take a cup of coffee and find some nice reading
(e.g., the [Patmos Reference Handbook](http://patmos.compute.dtu.dk/patmos_handbook.pdf)).


We can start with the standard, harmless looking Hello
World:

    main() {
        printf("Hello Patmos!\n");
    }

With the compiler installed it can be compiled to a Patmos executable
and run with the simulator as follows:

    patmos-clang hello.c
    pasim a.out

However, this innocent examples is quiet challenging for an embedded system.
For further details and how to build Patmos for an FPGA see Section 6 in the
[Patmos Reference Handbook](http://patmos.compute.dtu.dk/patmos_handbook.pdf).

You can also build the Patmos handbook yourself from the source.
You first need to install LaTeX (about 3 GB) with:

    sudo apt-get install texlive-full doxygen

The handbook is then built with:

    cd patmos/doc
    make


