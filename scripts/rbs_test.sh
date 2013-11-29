#!/bin/bash

BOOTAPP=bootable-cmp_hello
#BOOTAPP=localmem_load_store


# Compile the C program cmp_hello
make BOOTAPP=${BOOTAPP} bootcomp


# Assemble
#make BOOTAPP=${BOOTAPP} asm

# Generate a new patmos.v file
make BOOTAPP=${BOOTAPP} gen
