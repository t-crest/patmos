#!/bin/bash
make rom bsim APP=basic > ms.txt 2> tmp.txt
make hsim APP=basic 2> hs.txt > tmp.txt
echo basic
java -cp java/lib/patmos-tools.jar util.CompTest hs.txt ms.txt
