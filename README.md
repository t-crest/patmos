About this fork
============

This is a fork of patmos created to make a pull request to add Load-Linked
Store-Conditional scratchpad memory.

The hardware files are in the `conc` package. Some C example programs are
in the `llsc` app drectory. The new shared SPM is added with code 7.

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

    sudo apt-get install git default-jdk gitk cmake make g++ texinfo flex bison \
      subversion libelf-dev graphviz libboost-dev libboost-program-options-dev ruby-full \
      liblpsolve55-dev python zlib1g-dev gtkwave gtkterm scala

On a restricted machine (e.g. Cloud9) the bare minimum is:

    sudo apt-get install default-jdk git cmake make g++ texinfo flex bison \
      subversion libelf-dev graphviz libboost-dev libboost-program-options-dev ruby-full \
      python zlib1g-dev

Install sbt with:

    echo "deb https://dl.bintray.com/sbt/debian /" | sudo tee -a /etc/apt/sources.list.d/sbt.list
    sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 \
      --recv 2EE0EA64E40A89B84B2DF73499E82A75642AC823
    sudo apt-get update
    sudo apt-get install sbt

We assume that the T-CREST project will live in $HOME/t-crest.
Before building the compiler, add the path
to the compiler executables into your .bashrc or .profile:

    export PATH=$PATH:$HOME/t-crest/local/bin

Use an absolute path as LLVM cannot handle a path relative to the
home directory (~).

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


Known Issues
============

- [ ] `patmos-llvm` currently does not compile with clang > 3.4 on Ubuntu 15.04.
      As a workaround, uninstall `clang`, install `clang-3.4` and create symlinks
      `clang` and `clang++` to `clang-3.4` and `clang++-3.4`.
