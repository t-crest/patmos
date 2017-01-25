
WCET Hello World Example
========================

This folder includes a minimal example to explore worst-case execution time
(WCET) analysis with `platin` for Patmos.

To be able to analyze a program, the compiler needs ot be instructed to
ouput the program in .pml format during compilation:

    patmos-clang -O2 -mserialize=simple.pml simple.c

The WCET analysis with platin is excuted as follows:

    platin wcet -i simple.pml -b a.out -e foo --report

Those commands assume a standard configuration of Patmos that is
the default single core configuration with the DE2-115 memory timing.

The two commands and further commands to explore the result are
included in the `Makefile`.
