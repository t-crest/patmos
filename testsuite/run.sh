#!/bin/bash

tests="basic test load_store_stackcache ALU ALUi ALUl compare dual_forwarding dual_even_odd_address forward_issue unary load_store_data_cache load_store_scratchpad load_store_scratchpad_new load_store_scratchpad_new2 predication fetch_double ld_st_test branch predicated_predicate"
tests_c="hello_test"
not_working="non"
expect_fail=0

# How to implement timeout?
# Caveat: Neither timeout nor timeout --foreground work properly
timeout=""

make tools
make rom bsim

echo === Tests ===
failed=()
#for f in  ${tests_c}; do
#    $timeout testsuite/single_c.sh ${f}
#    result=$?
#   if [ "$result" -eq 124 ] ; then
#        echo " timeout"
#    fi
#    if [ "$result" -ne 0 ] ; then
#        failed+=("${f}")
#    fi
#done
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

