#!/bin/bash
tests="basic test ALU ALUi compare"
not_working="branch dual_forwarding dual_even_odd_address"
for f in  ${tests}; do
    testsuit/single.sh ${f}
done
