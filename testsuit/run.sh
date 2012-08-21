#!/bin/bash
make hsim APP=basic 2> hs.txt
make rom bsim APP=basic > ms.txt
java -cp java/lib/patmos-tools.jar util.CompTest hs.txt ms.txt
