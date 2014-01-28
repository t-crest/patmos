About this pull
============

This version of the Patmos source tree is the one currently used by York within the T-CREST
project. It contains a few hacks to (badly) expose a plain OcpBurst interface from the top
in order to connect it to the rest of the T-CREST platform.

This version is not intended to be merged into the main Patmos repository, since there are also
some miscallaneous changes to the build system (I don't have the dependencies for the simulator...)
and the external interface of Patmos is no longer compatible with the canonical copy.

About Patmos
============

Patmos is a time-predictable VLIW processor.
Patmos is the processor for the T-CREST project.
See also: http://www.t-crest.org/ and http://patmos.compute.dtu.dk/

The Patmos [Handbook]
(http://patmos.compute.dtu.dk/patmos_handbook.pdf)
is work in progress, but contains build instructions in Section 5.

For questions and discussions join the Patmos mailing list at:
http://tech.groups.yahoo.com/group/patmos-processor/

Getting Started
===============

Several packages need to be installed.
The following apt-get lists the packages that need to be
installed on a Ubuntu Linux:

    sudo apt-get install openjdk-7-jdk git cmake make g++ texinfo flex bison \
      subversion libelf-dev graphviz libboost-dev libboost-program-options-dev ruby1.9.1 \
      ruby1.9.1-dev python zlib1g-dev gtkwave

We assume that the T-CREST project will live in $HOME/t-crest.
Patmos and the compiler can be checked out from GitHub and are built as follows:

    mkdir ~/t-crest
    cd ~/t-crest
    git clone https://github.com/t-crest/patmos-misc.git misc
    ./misc/build.sh

For developers with push permission the ssh based clone string is:

    git clone git@github.com:t-crest/patmos-misc.git misc

build.sh will checkout several other repositories (the compiler, library,
the Patmos source, and benchmarks) and
build the compiler, the Patmos simulator, and the test benches.
Therefore, take a cup of coffee and find some nice reading
(e.g., the [Patmos Handbook] (http://patmos.compute.dtu.dk/patmos_handbook.pdf)).
After building the compiler, add the path
to the compiler executables into your .profile:

    export PATH=$PATH:$HOME/t-crest/local/bin

Use an absolute path as LLVM cannot handle a path relative to the
home directory (~).

One might start with the standard, harmless looking Hello World:

    main() {
        printf("Hello Patmos!\n");
    }

With the compiler installed it can be compiled to a Patmos executable
and run with the simulator as follows:

    patmos-clang hell.c
    pasim a.out

However, this innocent examples is quiet challenging for an embedded system.
For further details and how to build Patmos for an FPGA see:
[Patmos Handbook] (http://patmos.compute.dtu.dk/patmos_handbook.pdf).
