#!/bin/bash

tests="basic simple test load_store_stackcache ALU ALUi ALUl dual_forwarding dual_even_odd_address forward_issue unary load_store_data_cache load_store_scratchpad load_store_scratchpad_new load_store_scratchpad_new2 predication fetch_double  branch predicated_predicate"

tests_chsl="basic simple test load_store_stackcache ALU ALUi ALUl dual_forwarding dual_even_odd_address forward_issue unary load_store_data_cache load_store_scratchpad load_store_scratchpad_new load_store_scratchpad_new2 predication fetch_double  branch predicated_predicate"

tests_c="hello_test"
not_working="none"
not_working_chsl="none"
expect_fail=0
expect_fail_chsl=9

# How to implement timeout?
# Caveat: Neither timeout nor timeout --foreground work properly
timeout=""

make tools
make rom bsim

echo === VHDL Tests ===
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


make csim

echo === Chisel Tests ===
failed_chsl=()
for f in  ${tests_chsl}; do
    $timeout testsuite/single_chsl.sh ${f}
    result=$?
    if [ "$result" -eq 124 ] ; then
        echo " timeout"
    fi
    if [ "$result" -ne 0 ] ; then
        failed_chsl+=("${f}")
    fi
done

for f in  ${not_working_chsl}; do
    echo $f
    echo " skipped"
done
if [ "${#failed_chsl[@]}" -ne 0 ] ; then
    echo "Failed tests: ${failed_chsl[@]}" >&2
else
    echo "All tests ok"
fi

echo "Test VHDL failures: expected ${expect_fail}, actual ${#failed[@]}" >&2
echo "Test Chisel failures: expected ${expect_fail_chsl}, actual ${#failed_chsl[@]}" >&2
if [ "${#failed[@]}" -ne $expect_fail ] ; then
    exit 1
else
    if [ "${#failed_chsl[@]}" -ne $expect_fail_chsl ] ; then
        exit 1
    else
        exit 0
    fi
fi

