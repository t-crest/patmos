#!/bin/bash
make rom bsim APP=$1 > ms.txt 2> tmp.txt
make hsim APP=$1 2> hs.txt > tmp.txt
echo $1
java -cp java/lib/patmos-tools.jar util.CompTest hs.txt ms.txt
