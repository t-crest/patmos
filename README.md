About Patmos
============

Patmos is a time-predictable VLIW processor.
Patmos is the processor for the T-CREST project.
See also: http://www.t-crest.org/ and http://patmos.compute.dtu.dk/

The Patmos [Reference Handbook](http://patmos.compute.dtu.dk/patmos_handbook.pdf)
contains build instructions in Section 5.

For questions and discussions use the GitHub discussion area of Patmos at:
https://github.com/t-crest/patmos/discussions


Getting Started
===============

In the following the installation and of the T-CERST/Patmos tools and
design on a Linux machine is described.

## A Virtual Machine for Development

However, we also provide a
VMWare virtual machine with Ubuntu 20.04 and all tools installed and
compiled at:

 * [Patmos VM](https://patmos-download.compute.dtu.dk/patmos-training.zip)

The user id is ```patmos``` and the password is also ```patmos```.

## Linux (Ubuntu) based Installation

Several packages need to be installed.
The following apt-get lists the packages that need to be
installed on a Ubuntu Linux:

```
sudo apt-get install git openjdk-11-jdk gitk cmake make g++ texinfo flex bison \
  subversion libelf-dev graphviz libboost-dev libboost-program-options-dev ruby-full \
  liblpsolve55-dev zlib1g-dev gtkwave gtkterm scala autoconf libfl2 expect verilator curl
```

Install sbt according to the instructions from [sbt download](https://www.scala-sbt.org/1.x/docs/Installing-sbt-on-Linux.html).

Install Quartus as instructed in the Patmos Handbook build instructions chapter.

We assume that the T-CREST project will live in $HOME/t-crest.
Before building the compiler, add the path
to the compiler executables into your .bashrc or .profile:

    export PATH=$PATH:$HOME/t-crest/local/bin

Use an absolute path as LLVM cannot handle a path relative to the
home directory (~). Logout and login again to make your new PATH setting active.


Patmos and the compiler can be checked out from GitHub and are built as follows:

    mkdir ~/t-crest
    cd ~/t-crest
    git clone git@github.com:t-crest/patmos-misc.git misc
    ./misc/build.sh

Without a GitHub login the ssh based clone string is:

    git clone https://github.com/t-crest/patmos-misc.git misc

build.sh will checkout several other repositories (the compiler, library,
and the Patmos source) and build the compiler and the Patmos simulator.
Therefore, take a cup of coffee and find some nice reading
(e.g., the [Patmos Reference Handbook](http://patmos.compute.dtu.dk/patmos_handbook.pdf)).

You can also install (quicker) the precompiled tools with:

    ./misc/build.sh -q

## Hello World

We can start with the standard, harmless looking Hello
World:

    main() {
        printf("Hello Patmos!\n");
    }

With the compiler installed it can be compiled to a Patmos executable
and run with the sw simulator and the hardware emulation as follows:

    patmos-clang hello.c
    pasim a.out
    patemu a.out

However, this innocent examples is quite challenging for an embedded system.
For further details and how to build Patmos for an FPGA see Section 6 in the
[Patmos Reference Handbook](http://patmos.compute.dtu.dk/patmos_handbook.pdf).

You can also build the Patmos handbook yourself from the source.
You first need to install LaTeX (about 3 GB) with:

    sudo apt-get install texlive-full doxygen

The handbook is then built with:

    cd patmos/doc
    make

See also [TODO.md](TODO.md).
