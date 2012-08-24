#!/bin/bash
tests="basic test ALU ALUi compare"
not_working="branch dual_forwarding dual_even_odd_address"

make tools
make rom bsim
echo === Tests ===
for f in  ${tests}; do
    testsuite/single.sh ${f}
done
for f in  ${not_working}; do
    echo issue with ${f}
done
