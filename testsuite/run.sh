#!/bin/bash

tests="basic test ALU ALUi ALUl compare dual_forwarding dual_even_odd_address forward_issue unary load_store_data_cache load_store_scratchpad predication fetch_double"
not_working="fetch_double branch"
expect_fail=2

# How to implement timeout?
# Caveat: Neither timeout nor timeout --foreground work properly
timeout=""

make tools
make rom bsim

echo === Tests ===
failed=()
for f in  ${tests}; do
    $timeout testsuite/single.sh ${f}
    result=$?
    if [ "$result" -eq 124 ] ; then
        echo " timeout"
    fi
    if [ "$result" -ne 0 ] ; then
        failed+=("${f}")
    fi
done
for f in  ${not_working}; do
    echo $f
    echo " skipped"
done
if [ "${#failed[@]}" -ne 0 ] ; then
    echo "Failed tests: ${failed[@]}" >&2
else
    echo "All tests ok"
fi
echo "Test failures: expected ${expect_fail}, actual ${#failed[@]}" >&2
if [ "${#failed[@]}" -ne $expect_fail ] ; then
    exit 1
else
    exit 0
fi
