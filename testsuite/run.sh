#!/bin/bash
# Directories containing assembler tests
cd asm
test_dirs="./inst_tests ./vliw_tests"
#for dir in ${test_dirs} ; do
#    mkdir ../tmp/${dir} 2>/dev/null
#done
# Discovering the tests in the specified directories.
test_disc=" "
for td in ${test_dirs}; do
    for f in ${td}/* ; do
        file="$f"
        extension="${file##*.}"
        if [[ "$extension" == "s" ]]; then
            # Only files with .s extension are accepted as test cases
            test_disc+="${file%.*} "
        fi
    done
done
cd ..

tests="basic simple test load_store_stackcache ALU ALUi ALUl dual_forwarding scratchpad dual_even_odd_address forward_issue unary load_store_data_cache load_store_scratchpad load_store_scratchpad_new load_store_scratchpad_new2 predication fetch_double  branch predicate predicated_predicate"
tests+=${test_disc}


tests_c="hello_test"
not_working="none"
not_working_chsl="none"
expect_fail=7
expect_fail_chsl=4

# How to implement timeout? IMPLEMENTED!
# But does not work under OSX
timeout=90

function run {
    testsuite/single.sh $1
    result=$?
    if [ "$result" -ne 0 ] ; then
        failed+=("$1")
        echo "$1" > result.tmp
    else
        echo -ne "ok" > result.tmp
    fi
}

function wait_timeout {
    start=$(date +%s);
    runtime=$(($(date +%s)-$start))
    while (( ${runtime} < ${timeout} )); do
        sleep 2s
        if [[ "$(ps -e | grep $1)" == "" ]] ; then
            break;
        fi
        runtime=$(($(date +%s)-$start))
    done
    if (( ${runtime} >= ${timeout} )); then
        # This way of killing the simulator does NOT support multiple run.sh running in parallel
        kill $(ps -ef | grep pasim | grep -v grep | awk '{print $2}') 2>/dev/null
        # pkill -f pasim ## pkill not standard on Unix/Linux
        echo " timeout"
    fi
}

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

for f in ${tests}; do
    run ${f} &
    pid=$!
    wait_timeout ${pid}
    if [[ "$(cat result.tmp)" != "ok" ]]; then
        failed+=("$(cat result.tmp)")
    fi
done
rm result.tmp

for f in ${not_working} ;
do
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
for f in  ${tests}; do
    testsuite/single_chsl.sh ${f}
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

nr=`echo ${tests} | wc -w`
echo "Test VHDL failures: expected ${expect_fail}, actual ${#failed[@]} out of ${nr}" >&2
echo "Test Chisel failures: expected ${expect_fail_chsl}, actual ${#failed_chsl[@]} out of ${nr}" >&2
if [ "${#failed[@]}" -ne $expect_fail ] ; then
    exit 1
else
    if [ "${#failed_chsl[@]}" -ne $expect_fail_chsl ] ; then
        exit 1
    else
        exit 0
    fi
fi

