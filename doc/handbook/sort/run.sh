#!/bin/bash

mkdir -p tmp

platin pml-config --target patmos-unknown-unknown-elf -o config.pml -m 2k -M fifo8 

patmos-clang `platin tool-config -i config.pml -t clang` -O2 -o sort -mserialize=sort.pml sort.c

platin pml -i sort.pml --print-flowfacts | tee flowfacts.txt

platin wcet -i config.pml --enable-trace-analysis --enable-wca -b sort -e gen_sort -i sort.pml -o wcet.pml --outdir tmp --report report.txt

platin visualize -i wcet.pml -o tmp --show-timings -f gen_sort --graphviz-format=eps

