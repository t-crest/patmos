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

tests="basic minimal simple ALU ALUi ALUl compare dual_forwarding fetch_double dual_even_odd_address forward_issue ldst ld_st_test load_use load_store_stackcache spill sspill load_store_data_cache load_store_scratchpad load_store_scratchpad_new load_store_scratchpad_new2 scratchpad predication branch predicate predicated_predicate pred_issue call callr mulpipe mfsmts test test_asm test_case_plan test_mfs test_mts test_datacache"
tests+=${test_disc}

not_working_chsl="none"
expect_fail_chsl=0

# How to implement timeout? IMPLEMENTED!
# But does not work under OSX
timeout=90

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

function run_chsl {
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
}

function run_isa {
	echo === ISA Tests ===
	failed_isa=()
	for f in  ${tests}; do
		testsuite/single_isa.sh ${f}
		result=$?
		if [ "$result" -eq 124 ] ; then
			echo " timeout"
		fi
		if [ "$result" -ne 0 ] ; then
			failed_isa+=("${f}")
		fi
	done

	for f in  ${not_working_isa}; do
		echo $f
		echo " skipped"
	done
	if [ "${#failed_isa[@]}" -ne 0 ] ; then
		echo "Failed tests: ${failed_isa[@]}" >&2
	else
		echo "All tests ok"
	fi
}

function run_all {

	run_isa

	run_chsl

	nr=`echo ${tests} | wc -w`
	echo "Test Chisel failures: expected ${expect_fail_chsl}, actual ${#failed_chsl[@]} out of ${nr}" >&2
	if [ "${#failed_chsl[@]}" -ne $expect_fail_chsl ] ; then
		exit 1
	else
		exit 0
	fi
}

make tools
run_all
